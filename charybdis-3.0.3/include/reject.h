/*
 *  ircd-ratbox: A slightly useful ircd
 *  reject.h: header to a file which rejects users with prejudice
 *
 *  Copyright (C) 2003 Aaron Sethman <androsyn@ratbox.org>
 *  Copyright (C) 2003-2004 ircd-ratbox development team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 *
 *  $Id: reject.h 3446 2007-05-14 22:21:16Z jilles $
 */
#ifndef INCLUDED_reject_h
#define INCLUDED_reject_h

/* amount of time to delay a rejected clients exit */
#define DELAYED_EXIT_TIME	10

extern rb_dlink_list delay_exit;

void init_reject(void);
int check_reject(struct Client *);
void add_reject(struct Client *, const char *mask1, const char *mask2);
void flush_reject(void);
int remove_reject_ip(const char *ip);
int remove_reject_mask(const char *mask1, const char *mask2);

int add_unknown_ip(struct Client *client_p);
void del_unknown_ip(struct Client *client_p);

#endif

