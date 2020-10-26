#pragma once

enum class ESpellSlot
{
	Q = 0,
	W = 1,
	E = 2,
	R = 3,
	D = 4,
	F = 5,
	Item1 = 6,
	Item2 = 7,
	Item3 = 8,
	Item4 = 9,
	Item5 = 10,
	Item6 = 11,
	Trinket = 12,
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

