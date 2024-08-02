#include "Moveorder.h"

#include "consts.h"
#include "tt.h"

Movelist orderMoves(Movelist moveList, Hash* entry)
{
	if (entry == nullptr)
	{
		return moveList;
	}

	Move hashMove = entry->move;

	if (hashMove != Move::NULL_MOVE)
	{
		for (Move move : moveList)
		{
			if (move == hashMove)
			{
				Move cache = moveList.front();
				moveList.front() = hashMove;
				moveList.add(cache);
				//std::cout << "hash move is: " << hashMove << "    the front is " << moveList.front() << std::endl;
				break;
			}
		}
	}
	return moveList;
}