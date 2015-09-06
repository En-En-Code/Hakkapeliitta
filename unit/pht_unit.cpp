/*
    Hakkapeliitta - A UCI chess engine. Copyright (C) 2013-2015 Mikko Aarnos.

    Hakkapeliitta is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Hakkapeliitta is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Hakkapeliitta. If not, see <http://www.gnu.org/licenses/>.
*/

#include "..\src\pht.hpp"
#include <boost\test\unit_test.hpp>

BOOST_AUTO_TEST_CASE(AllCasesPHT)
{
    PawnHashTable pht;
    Bitboard passers;
    std::array<uint8_t, 2> pawnShelterScore;
    int scoreOp, scoreEd;

    pht.save(5270488176186631498, 1024, 15, 20, { 5, 7 });

    BOOST_CHECK(pht.probe(5270488176186631498, passers, scoreOp, scoreEd, pawnShelterScore));
    BOOST_CHECK(passers == 1024);
    BOOST_CHECK(scoreOp == 15);
    BOOST_CHECK(scoreEd == 20);
    BOOST_CHECK(pawnShelterScore[Color::White] == 5);
    BOOST_CHECK(pawnShelterScore[Color::Black] == 7);

    pht.clear();
    BOOST_CHECK(!pht.probe(5270488176186631498, passers, scoreOp, scoreEd, pawnShelterScore));
}



