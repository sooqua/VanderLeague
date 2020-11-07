#include "definitions.h"
#include "ioctls.h"
#include "ntddk.h"

#define drv_device L"\\Device\\injdrv"
#define drv_dos_device L"\\DosDevices\\injdrv"
#define drv  L"\\Driver\\injdrv"

PDEVICE_OBJECT driver_object;
UNICODE_STRING dev, dos;

HANDLE processId;
UINT64 tablemodule, protected_module;

void unload_driver(PDRIVER_OBJECT DriverObject);
NTSTATUS ioctl_create(PDEVICE_OBJECT device, PIRP irp); 
NTSTATUS ioctl_close(PDEVICE_OBJECT device, PIRP irp);
NTSTATUS io_device_control(PDEVICE_OBJECT device, PIRP Irp);
__inline NTSTATUS copy_memory(PEPROCESS src_proc, PEPROCESS target_proc, PVOID src, PVOID dst, SIZE_T size);
UINT64 get_module_handle(UINT64 pid, LPCWSTR module_name);
void PloadImageNotifyRoutine(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo);
NTSTATUS write_protected_memory(PVOID src, PVOID dst, SIZE_T size);

void PloadImageNotifyRoutine(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo) {
	if (FullImageName == NULL) {
		return;
	}

	if (wcsstr(FullImageName->Buffer, L"League of Legends.exe")) {
		//DbgPrintEx(0, 0, "Found process ID: %p \n", ProcessId);
		processId = ProcessId;
	}
	else if (wcsstr(FullImageName->Buffer, L"d3d9.dll") && processId == ProcessId) {
		//DbgPrintEx(0, 0, "Found table module: %p\n", ImageInfo->ImageBase);
		tablemodule = (UINT64)(ULONG_PTR)ImageInfo->ImageBase;
	}
}

NTSTATUS init(PDRIVER_OBJECT driver, PUNICODE_STRING path) { 
	RtlInitUnicodeString(&dev, drv_device);
	RtlInitUnicodeString(&dos, drv_dos_device);

	IoCreateDevice(driver, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &driver_object);
	IoCreateSymbolicLink(&dos, &dev);

	driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = io_device_control;
	driver->MajorFunction[IRP_MJ_CREATE] = ioctl_create;
	driver->MajorFunction[IRP_MJ_CLOSE] = ioctl_close;
	driver->DriverUnload = &unload_driver;

	PsSetLoadImageNotifyRoutine(PloadImageNotifyRoutine);

	driver_object->Flags |= DO_DIRECT_IO;
	driver_object->Flags &= ~DO_DEVICE_INITIALIZING;

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver,	PUNICODE_STRING path) { 
	NTSTATUS status;
	UNICODE_STRING drv_name;
	RtlInitUnicodeString(&drv_name, drv);
	status = IoCreateDriver(&drv_name, &init);
	return status;
}
 
NTSTATUS io_device_control(PDEVICE_OBJECT device, PIRP irp){
	NTSTATUS status;
	ULONG_PTR info_size = 0; 
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
	UINT64 control_code = stack->Parameters.DeviceIoControl.IoControlCode;
	
	switch (control_code) {
	case ioctl_allocate_virtual_memory: {
			pk_alloc_mem_request in = (pk_alloc_mem_request)irp->AssociatedIrp.SystemBuffer;
			PEPROCESS target_proc;
			status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)in->pid, &target_proc);
			if (NT_SUCCESS(status)) {
				KAPC_STATE apc;
				KeStackAttachProcess(target_proc, &apc);
				status = ZwAllocateVirtualMemory(ZwCurrentProcess(), (PVOID*)&(ULONG_PTR)in->addr, 0, (PSIZE_T)&(ULONG_PTR)in->size, (ULONG)(ULONG_PTR)in->allocation_type, (ULONG)(ULONG_PTR)in->protect);
				KeUnstackDetachProcess(&apc);
				ObfDereferenceObject(target_proc);
			}
			info_size = sizeof(k_alloc_mem_request);
		} break;

	case ioctl_protect_virutal_memory: {
			pk_protect_mem_request in = (pk_protect_mem_request)irp->AssociatedIrp.SystemBuffer;
			PEPROCESS target_proc;
			status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)in->pid, &target_proc);
			if (NT_SUCCESS(status)) {
				KAPC_STATE apc;
				ULONG old_protection;
				KeStackAttachProcess(target_proc, &apc);
				status = ZwProtectVirtualMemory(ZwCurrentProcess(), (PVOID*)&(ULONG_PTR)in->addr, (PSIZE_T)&(ULONG_PTR)in->size, (ULONG)(ULONG_PTR)in->protect, (PULONG)(ULONG_PTR)&old_protection);
				KeUnstackDetachProcess(&apc);
				in->protect = old_protection;
				ObfDereferenceObject(target_proc);
			}
			info_size = sizeof(k_protect_mem_request);
		} break;
	
	case ioctl_read_memory: {
			pk_rw_request in = (pk_rw_request)irp->AssociatedIrp.SystemBuffer;
			PEPROCESS target_proc; 
			status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)in->pid, &target_proc);
			if (NT_SUCCESS(status)) {
				status = copy_memory(PsGetCurrentProcess(), target_proc, (PVOID)(ULONG_PTR)in->src, (PVOID)(ULONG_PTR)in->dst, (SIZE_T)(ULONG_PTR)in->size);
				ObfDereferenceObject(target_proc);
			} 
			info_size = sizeof(k_rw_request);
		} break;

	case ioctl_write_memory: {
			pk_rw_request in = (pk_rw_request)irp->AssociatedIrp.SystemBuffer;
			PEPROCESS target_proc; 
			status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)in->pid, &target_proc);
			if (NT_SUCCESS(status)) {
				status = copy_memory(target_proc, PsGetCurrentProcess(), (PVOID)(ULONG_PTR)in->src, (PVOID)(ULONG_PTR)in->dst, (SIZE_T)(ULONG_PTR)in->size);
				ObfDereferenceObject(target_proc);
			}
			info_size = sizeof(k_rw_request);
		} break;

	case ioctl_get_module_base: {
			pk_get_base_module_request in = (pk_get_base_module_request)irp->AssociatedIrp.SystemBuffer;
			UINT64 handle = get_module_handle(in->pid, in->name);
			in->handle = handle;
			status = STATUS_SUCCESS;
			info_size = sizeof(k_get_base_module_request);
	} break;

	case ioctl_get_table_module_base: {
		pk_get_table_module_base_request in = (pk_get_table_module_base_request)irp->AssociatedIrp.SystemBuffer;
		in->handle = tablemodule;
		status = STATUS_SUCCESS;
		info_size = sizeof(k_get_table_module_base_request);
	} break;

	case ioctl_write_protected_memory: {
		pk_rw_protected_request in = (pk_rw_protected_request)irp->AssociatedIrp.SystemBuffer;
		PEPROCESS target_proc;
		status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)in->pid, &target_proc);
		if (NT_SUCCESS(status)) {
			KAPC_STATE apc;
			KeStackAttachProcess(target_proc, &apc);
			status = write_protected_memory((PVOID)(ULONG_PTR)in->src, (PVOID)(ULONG_PTR)in->dst, (SIZE_T)(ULONG_PTR)in->size);
			KeUnstackDetachProcess(&apc);
			ObfDereferenceObject(target_proc);
		}
		info_size = sizeof(pk_rw_protected_request);
	} break;

	default:
			status = STATUS_INVALID_PARAMETER;
			info_size = 0;
		break;
	}

	irp->IoStatus.Status = status;
	irp->IoStatus.Information = info_size;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

void unload_driver(PDRIVER_OBJECT DriverObject) {
	PsRemoveLoadImageNotifyRoutine(PloadImageNotifyRoutine);
	IoDeleteSymbolicLink(&dos);
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS ioctl_create(PDEVICE_OBJECT device, PIRP irp) {
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS ioctl_close(PDEVICE_OBJECT device, PIRP irp) {
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

__inline NTSTATUS copy_memory(PEPROCESS src_proc, PEPROCESS target_proc, PVOID src, PVOID dst, SIZE_T size) {
	SIZE_T bytes;
	return MmCopyVirtualMemory(target_proc, src, src_proc, dst, size, UserMode, &bytes);
}

UINT64 get_module_handle(UINT64 pid, LPCWSTR module_name) {
	PEPROCESS target_proc;
	UINT64 base = 0;
	if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)pid, &target_proc)))
		return 0;

	KeAttachProcess((PKPROCESS)target_proc);

	PPEB peb = PsGetProcessPeb(target_proc);
	if (!peb)
		goto end;

	if (!peb->Ldr || !peb->Ldr->Initialized)
		goto end;


	UNICODE_STRING module_name_unicode;
	RtlInitUnicodeString(&module_name_unicode, module_name);
	for (PLIST_ENTRY list = peb->Ldr->InLoadOrderModuleList.Flink;
		list != &peb->Ldr->InLoadOrderModuleList;
		list = list->Flink) {
		PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		if (RtlCompareUnicodeString(&entry->BaseDllName, &module_name_unicode, TRUE) == 0) {
			base = (UINT64)(ULONG_PTR)entry->DllBase;
			goto end;
		}
	}

end:
	KeDetachProcess();
	ObDereferenceObject(target_proc);
	return base;
}

NTSTATUS KernelMemCopy(void* dst, void* src, SIZE_T size)
{
	NTSTATUS status = STATUS_SUCCESS;
	__try
	{
		KIRQL irql;
		KIRQL curreirql;

		curreirql = KeGetCurrentIrql();
		if (curreirql < DISPATCH_LEVEL)
		{
			irql = KeRaiseIrqlToDpcLevel();
		}
		UINT64 cr0 = __readcr0();
		cr0 &= 0xfffffffffffeffff;
		__writecr0(cr0);
		_disable();
		ProbeForRead(src, size, sizeof(UINT8));
		ProbeForWrite(dst, size, sizeof(UINT8));
		RtlCopyMemory(dst, src, size);
		cr0 = __readcr0();
		cr0 |= 0x10000;
		_enable();
		__writecr0(cr0);
		if (curreirql < DISPATCH_LEVEL)
		{
			KeLowerIrql(irql);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrintEx(0, 0, "Driver EXCEPTION CAUGHT\n");
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}


NTSTATUS write_protected_memory(PVOID src, PVOID dst, SIZE_T size) {
	NTSTATUS status = STATUS_SUCCESS;

	status = KernelMemCopy(dst, src, size);

	return status;

	//PAGED_CODE();
	//NTSTATUS Status = STATUS_SUCCESS;

	//PMDL Mdl = IoAllocateMdl(address, size, FALSE, FALSE, NULL);
	//if (!Mdl) {
	//	DbgPrintEx(0, 0, "IoAllocateMdl FAILED\n");
	//	Status = STATUS_INSUFFICIENT_RESOURCES;
	//	return Status;
	//}

	//try
	//{
	//	//
	//	// Probe and lock the pages of this buffer in physical memory.
	//	// You can specify IoReadAccess, IoWriteAccess or IoModifyAccess
	//	// Always perform this operation in a try except block.
	//	//  MmProbeAndLockPages will raise an exception if it fails.
	//	//
	//	MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess);
	//}
	//except(EXCEPTION_EXECUTE_HANDLER)
	//{
	//	DbgPrintEx(0, 0, "MmProbeAndLockPages FAILED\n");
	//	Status = GetExceptionCode();
	//	IoFreeMdl(Mdl);
	//	return Status;
	//}

	//PVOID Mapping = MmMapLockedPagesSpecifyCache(Mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
	//if (!Mapping) {
	//	DbgPrintEx(0, 0, "MmMapLockedPagesSpecifyCache FAILED\n");
	//	Status = STATUS_INSUFFICIENT_RESOURCES;
	//	IoFreeMdl(Mdl);
	//	return Status;
	//}

	//MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);

	//// Write your buffer to mapping:
	//RtlCopyMemory(Mapping, (PVOID)buffer, size);

	//// Resources freeing:
	//MmUnmapLockedPages(Mapping, Mdl);
	//MmUnlockPages(Mdl);
	//IoFreeMdl(Mdl);

	//return Status;
}