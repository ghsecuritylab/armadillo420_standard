/* 
 * $Id: timer.h,v 1.8 2003/03/12 12:50:02 andrei Exp $
 *
 *
 * Copyright (C) 2001-2003 Fhg Fokus
 *
 * This file is part of ser, a free SIP server.
 *
 * ser is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * For a license to use the ser software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * ser is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef _PIKE_TIMER_H
#define _PIKE_TIMER_H



struct pike_timer_link {
	int timeout;
	struct pike_timer_link *next;
	struct pike_timer_link *prev;
};

struct pike_timer_head {
	struct pike_timer_link *first;
	struct pike_timer_link *last;
};

void append_to_timer(struct pike_timer_head*, struct pike_timer_link* );
void remove_from_timer(struct pike_timer_head*, struct pike_timer_link* );
int  is_empty(struct pike_timer_head*);
struct pike_timer_link *check_and_split_timer(struct pike_timer_head*,int);

#endif

