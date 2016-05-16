/*
 *      This file is part of the SmokeOS project.
 *  Copyright (C) 2015  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Usermode CPU wrapper implementation.
 */

int testCase (const char *dir);


#include <stdlib.h>

/* At this point we leave CRTK. */
int main (int argc, char** argv)
{
  int until = 0;
  until = until || testCase ("fs1");
  until = until || testCase ("base");
  // until = until || testCase ("mthread");
  until = until || testCase ("mmap");
  until = until || testCase ("base");
  until = until || testCase ("shell");

  return until;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
