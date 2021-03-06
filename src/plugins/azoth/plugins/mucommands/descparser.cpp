/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "descparser.h"
#include <QApplication>
#include <QtDebug>
#include <interfaces/azoth/iprovidecommands.h>

namespace LC
{
namespace Azoth
{
namespace MuCommands
{
	DescParser::DescParser ()
	{
		Cmd2Desc_ =
		{
			{
				"/names",
				{
					tr ("Shows the nick names of multiuser chat room participants."),
					""
				}
			},
			{
				"/urls",
				{
					tr ("Lists the URLs appeared in the current chat session."),
					tr (R"(Usage: @/urls@

Returns the ordered list of all URLs that appeared in the current chat session. Duplicates are omitted. The indexes of the URLs can be passed as parameters to @/openurl@ and @/fetchurl@ commands.)")
				}
			},
			{
				"/openurl",
				{
					tr ("Opens an URL from the current chat session by its index."),
					tr (R"(Usage: @/openurl@ _[idx1]_ _[idx2]_ ...

Opens the URLs identified by their indexes in the list returned by the @/urls@ command. Multiple indexes can be given. If no indexes are given, the last URL is opened.)")
				}
			},
			{
				"/fetchurl",
				{
					tr ("Downloads an URL from the current chat session by its index."),
					tr (R"(Usage: @/fetchurl@ _[idx1]_ _[idx2]_ ...

Downloads the URLs identified by their indexes in the list returned by the @/urls@ command. Multiple indexes can be given. If no indexes are given, the last URL is downloaded.)")
				}
			},
			{
				"/vcard",
				{
					tr ("Shows VCards for the given participants, or a roster entry, or any protocol user identified by their ID."),
					tr (R"(Usage: @/vcard@ _[id1]_
_[id2]_...

Shows (and fetches, if required) VCards for the users identified by _id_. Multiple IDs could be given, separated by line breaks.

An ID is resolved in the following way:
# If the command is issued in a multiuser chat room, the ID is matched against the nicknames of the participants on the room.
# The ID is matched against names and protocol-specific IDs (like JIDs) of participants in the roster.
# If the protocol plugin supports that, the ID is resolved as a global entry ID in the protocol, even if it is not present in the roster.

If no ID is given, the author of last message is considered in case of a multiuser chat, or the current contact in case of a standard private chat.)")
				}
			},
			{
				"/version",
				{
					tr ("Shows the information about a participant or contact's software."),
					tr (R"(Usage: @/version@ _[id1]_
_[id2]_...

See the documentation for the @/vcard@ command regarding resolving IDs.)")
				}
			},
			{
				"/time",
				{
					tr ("Shows the current time of a participant or a contact or a remote ID."),
					tr (R"(Usage: @/time@ _[id1]_
_[id2]_...

For each ID, their respective timezone, the local time in their timezone and their UTC time is returned.

See the documentation for the @/vcard@ command regarding resolving IDs.)")
				}
			},
			{
				"/disco",
				{
					tr ("Opens the service discovery tab for the given participant or ID."),
					tr (R"(Usage: @/disco@ _[id1]_
_[id2]_...

See the documentation for the @/vcard@ command regarding resolving IDs.)")
				}
			},
			{
				"/nick",
				{
					tr ("Changes the nickname used in the current multiuser chat room."),
					tr (R"(Usage: @/nick@ _new nick_)")
				}
			},
			{
				"/subject",
				{
					tr ("Changes the subject in the current multiuser chat room."),
					tr (R"(Usage: @/subject@ _The new room subject._

The subject may span multiple lines.

Please note that in most protocols there is no reliable way to know if MUC subject may be changed in advance. Thus, this command will try its best to guess if subject change is allowed, but it still may silently fail.)")
				}
			},
			{
				"/join",
				{
					tr ("Joins a room."),
					tr (R"(Usage: @/join@ _room_ [_password_]

Joins a multiuser chat room identified by _room_, optionally using the given _password_.

In case of XMPP the _room_ may be either the full room ID, like @leechcraft\@conference.leechcraft.org@, or just the room name, like @leechcraft@. In the latter case the server name is resolved as following:
# If the current chat is related to a multiuser chat room or a private chat in a multiuser chat room, then this room's server is used.
# Otherwise, the server of the account to which the current chat is related is used, prepended with the @conference.@ string.)")
				}
			},
			{
				"/leave",
				{
					tr ("Leaves the current multiuser chat room."),
					tr (R"(Usage: @/leave@ _[leave message]_)")
				}
			},
			{
				"/rejoin",
				{
					tr ("Leaves the current multiuser chat room and then joins it again."),
					tr (R"(Usage: @/rejoin@ _[leave message]_)")
				}
			},
			{
				"/ping",
				{
					tr ("Pings the given contact and shows the round trip time of the ping request."),
					tr (R"(Usage: @/ping@ _[id1]_
_[id2]_ ...

See the documentation for the @/vcard@ command regarding resolving IDs.)")
				}
			},
			{
				"/last",
				{
					tr ("Shows the time of last activity, or connection, or uptime."),
					tr (R"(Usage: @/last@ _[id1]_
_[id2]_ ...

See the documentation for the @/vcard@ command regarding resolving IDs.

If the resolved ID is a contact, then either last activity time or last connection time are reported, depending on the protocol pecularities. If the resolved ID is a server, its uptime is returned.)")
				}
			},
			{
				"/invite",
				{
					tr ("Invites a roster entry into a multiuser chat room."),
					tr (R"(Usage: @/invite@ _id_ _[reason]_

If the command is issued in a private chat, the _id_ is interpreted as a MUC room ID, and the other part of the current private chat is invited to that MUC.

If the command is issued in a MUC, the _id_ is resolved just like in @/vcard@ case, and the resolved entry is invited to the current MUC.)")
				}
			},
			{
				"/pm",
				{
					tr ("Sends a private message to a participant of the current multiuser chat room."),
					tr (R"(Usage: @/pm@ _nickname_
_Message text_

The text can consist of multiple lines, depending on the protocol.)")
				}
			},
			{
				"/whois",
				{
					tr ("Shows the real ID of a multiuser chat participant, if available."),
					tr (R"(Usage: @/whois@ _nickname_)")
				}
			},
			{
				"/listperms",
				{
					tr ("Lists the available permission classes and roles for the current multiuser chat room."),
					tr (R"(Usage: @/listperms@

This command shows the list of available permission classes (like role and affiliation in case of XMPP) for the current multiuser chat room, as well as the available permission values for each of the classes.

The listed classes and values can be passed to the @/setperm@ command.

@/listperms@ can be issued both in the room itself and in any private chat with one of its participants.)")
				}
			},
			{
				"/setperm",
				{
					tr ("Sets the permissions (like affiliation or role in case of XMPP) of a given participant in a multiuser chat room."),
					tr (R"(Usage: @/setperm@ _permclass_ _perm_ &lt;@nick@|@id@&gt; &lt;_nick_|_id_&gt;
_[reason]_

It should be explicitly stated whether currently present participant nickname is mentioned or some protocol-defined ID string (like a JID in case of XMPP). The @nick@ argument is used in the former case, and @id@ — in the latter.

The list of available permission classes and permissions for a given protocol may be obtained via the @/listperms@ command.

Please note that you may type only first few letters of a permission class and value for _permclass_ and _perm_ parameters respectively, if only one string starts with the corresponding letters.

@/setperm@ can be issued both in the room itself and in any private chat with one of its participants.)")
				}
			},
			{
				"/kick",
				{
					tr ("Kicks the given participant from the current multiuser chat room."),
					tr (R"(Usage: @/kick@ _nickname_[|_reason_]

This is a shortcut for kicking the given participant by its nickname.)")
				}
			},
			{
				"/ban",
				{
					tr ("Bans the given participant from the current multiuser chat room."),
					tr (R"(Usage: @/ban@ _nickname_[|_reason_]

This is a shortcut for ban the given participant by its nickname.)")
				}
			},
			{
				"/subst",
				{
					tr ("Substitutes one string with another in a single outgoing message."),
					tr (R"(Usage: @/subst@ _pattern_ _replacement_ _message_

Replaces all occurences of _pattern_ with _replacement_ in _message_ and sends it. _message_ may span multiple lines, while _pattern_ and _replacement_ should NOT contain spaces.)")
				}
			}
		};
	}

	void DescParser::operator() (StaticCommand& cmd) const
	{
		if (!Cmd2Desc_.contains (cmd.Names_.first ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "no description for"
					<< cmd.Names_;
			return;
		}
		const auto& desc = Cmd2Desc_.value (cmd.Names_.first ());
		cmd.Description_ = desc.Description_;
		cmd.Help_ = desc.Help_;
	}
}
}
}
