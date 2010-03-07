/*
 *  Charybdis: an advanced Internet Relay Chat Daemon(ircd).
 *  m_omode.c: allows oper mode hacking
 *
 *  Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center
 *  Copyright (C) 1996-2002 Hybrid Development Team
 *  Copyright (C) 2002-2004 ircd-ratbox development team
 *  Copyright (C) 2006 Charybdis development team
 *  Copyright (C) 2010 IRCd-Damon Development Team 
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
 *  $Id: m_omode.c 3121 2007-01-02 13:23:04Z jilles $
 */

#include "stdinc.h"
#include "channel.h"
#include "client.h"
#include "hash.h"
#include "match.h"
#include "ircd.h"
#include "numeric.h"
#include "s_user.h"
#include "s_conf.h"
#include "s_newconf.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "packet.h"

static int mo_qmode(struct Client *, struct Client *, int, const char **);

struct Message qmode_msgtab = {
	"QMODE", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_oper, mg_ignore, mg_ignore, mg_ignore, {mo_qmode, 3}}
};

mapi_clist_av1 qmode_clist[] = { &qmode_msgtab, NULL };

DECLARE_MODULE_AV1(qmode, NULL, NULL, qmode_clist, NULL, NULL, "$Revision: 3121 $");

/*
 * mo_omode - MODE command handler
 * parv[0] - sender
 * parv[1] - channel
 */
static int
mo_qmode(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Channel *chptr = NULL;
	struct membership *msptr;
	char params[512];
	int i;
	int wasonchannel;

	/* admins only */
	if(!IsOwnerAdmin(source_p))
	{
		sendto_one(source_p, form_str(ERR_NOPRIVS), me.name, source_p->name, "admin");
		return 0;
	}

	/* Now, try to find the channel in question */
	if(!IsChanPrefix(parv[1][0]) || !check_channel_name(parv[1]))
	{
		sendto_one_numeric(source_p, ERR_BADCHANNAME,
				form_str(ERR_BADCHANNAME), parv[1]);
		return 0;
	}

	chptr = find_channel(parv[1]);

	if(chptr == NULL)
	{
		sendto_one_numeric(source_p, ERR_NOSUCHCHANNEL,
				form_str(ERR_NOSUCHCHANNEL), parv[1]);
		return 0;
	}

	/* Now know the channel exists */
	msptr = find_channel_membership(chptr, source_p);
	wasonchannel = msptr != NULL;

	if (is_chanowner(msptr))
	{
		sendto_one_notice(source_p, ":Use a normal MODE you idiot");
		return 0;
	}

	params[0] = '\0';
	for (i = 2; i < parc; i++)
	{
		if (i != 2)
			rb_strlcat(params, " ", sizeof params);
		rb_strlcat(params, parv[i], sizeof params);
	}

	sendto_wallops_flags(UMODE_WALLOP, &me, 
			     "QMODE called for [%s] [%s] by %s!%s@%s",
			     parv[1], params, source_p->name, source_p->username, source_p->host);
	ilog(L_MAIN, "QMODE called for [%s] [%s] by %s",
	     parv[1], params, get_owner_name(source_p));

	if(*chptr->chname != '&')
		sendto_server(NULL, NULL, NOCAPS, NOCAPS, 
			      ":%s WALLOPS :QMODE called for [%s] [%s] by %s!%s@%s",
			      me.name, parv[1], params, source_p->name, source_p->username,
			      source_p->host);

#if 0
	set_channel_mode(client_p, source_p->servptr, chptr, msptr, 
			 parc - 2, parv + 2);
#else
	if (parc == 4 && !strcmp(parv[2], "+q") && !irccmp(parv[3], source_p->name))
	{
		/* Opping themselves */
		if (!wasonchannel)
		{
			sendto_one_numeric(source_p, ERR_USERNOTINCHANNEL,
					   form_str(ERR_USERNOTINCHANNEL), parv[3], chptr->chname);
			return 0;
		}
		sendto_channel_local(ALL_MEMBERS, chptr, ":%s MODE %s +q %s",
				me.name, parv[1], source_p->name);
		sendto_server(NULL, chptr, CAP_TS6, NOCAPS,
				":%s TMODE %ld %s +q %s",
				me.id, (long) chptr->channelts, parv[1],
				source_p->id);
		sendto_server(NULL, chptr, NOCAPS, CAP_TS6,
				":%s MODE %s +q %s",
				me.name, parv[1], source_p->name);
		msptr->flags |= CHFL_CHANOWNER;
	}
	else
	{
		/* Hack it so set_channel_mode() will accept */
		if (wasonchannel)
			msptr->flags |= CHFL_CHANOWNER;
		else
		{
			add_user_to_channel(chptr, source_p, CHFL_CHANOWNER);
			msptr = find_channel_membership(chptr, source_p);
		}
		set_channel_mode(client_p, source_p, chptr, msptr, 
				parc - 2, parv + 2);
		/* We know they were not opped before and they can't have opped
		 * themselves as set_channel_mode() does not allow that
		 * -- jilles */
		if (wasonchannel)
			msptr->flags &= ~CHFL_CHANOWNER;
		else
			remove_user_from_channel(msptr);
	}
#endif
	return 0;
}
