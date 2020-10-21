enum class ESpellSlot
{
	Q = 0,
	W = 1,
	E = 2,
	R = 3,
	D = 4,
	F = 5,
	Recall = 13
};


enum class SpellState
{
	Ready = 0,
	NotAvailable = 4,
	Surpressed = 8,
	NotLearned = 12,
	Processing = 24,
	Cooldown = 32,
	NoMana = 64,
	Unknown
};

