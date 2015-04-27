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
 *      User ressources, and security managment.
 */
#include <smkos/kernel.h>
#include <smkos/kstruct/user.h>

/* ----------------------------------------------------------------------- */
kUser_t *search_user (const char *name, const char *domain)
{
  kUser_t *user;
  ll_for_each (&kSYS.userList_, user, kUser_t, allNd_) {
    if (strcmp (user->name_, name) == 0)
      return user;
  }

  __unused(domain);
  return NULL;
}


/* ----------------------------------------------------------------------- */
kUser_t *create_user(const char* username, int capacity)
{
  kUser_t *user = search_user(username, NULL);
  if (user != NULL)
    return __seterrnoN(EEXIST, kUser_t);

  user = KALLOC(kUser_t);
  user->name_ = strdup (username);
  user->capacity_ = capacity | CAP_EVERYBODY;
  ll_push_back(&kSYS.userList_, &user->allNd_);
  return user;
}


/* ----------------------------------------------------------------------- */
void destroy_user (kUser_t *user)
{
  assert (user->processCount_ == 0);
  kfree((char *)user->name_);
  kfree(user);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
