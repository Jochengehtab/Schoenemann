/*
  This file is part of the Schoenemann chess engine written by Jochengehtab

  Copyright (C) 2024-2025 Jochengehtab

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "tt.h"

void tt::storeEvaluation(std::uint64_t key, short depth, short type, int score, Move move, int eval)
{
	std::uint64_t index = key % size;

	// Get the HashNode
	Hash *node = table + index;

	// Store the entry
	node->setEntry(key, depth, type, score, move, eval);
}

Hash *tt::getHash(std::uint64_t zobristKey)
{
	// Gets the index based on the zobrist key
	std::uint64_t index = zobristKey % size;

	// Getting the node by the index
	Hash *node = table + index;

	// Check if we got the key in our Hash
	if (node->key == zobristKey)
	{
		return node;
	}

	// Returns a nullptr if nothing was found in the hash
	return nullptr;
}
void tt::clear()
{
	memset(static_cast<void *>(table), 0, size * sizeof(Hash));
}

void tt::init(std::uint64_t MB)
{
	std::uint64_t bytes = MB << 20;
	std::uint64_t maxSize = bytes / sizeof(Hash);

	size = 1;
	while (size <= maxSize)
	{
		size <<= 1;
	}

	size >>= 1;

	table = (Hash *)calloc(size, sizeof(Hash));
	clear();
}

std::uint64_t tt::getSize() const
{
	return size;
}

void tt::setSize(std::uint64_t MB)
{
	free(table);
	init(MB);
}

int tt::estimateHashfull() const
{
	int used = 0;

	return used;
}

tt::tt(std::uint64_t MB)
{
	init(MB);
}

tt::~tt()
{
	free(table);
}
