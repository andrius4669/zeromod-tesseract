#ifndef Z_MSGFILTER_H
#define Z_MSGFILTER_H 1

// message checking for editmute and flood protection
bool allowmsg(clientinfo *ci, clientinfo *cq, int type)
{
    if(!ci || ci->local) return true;
    switch(type)
    {
        case N_EDITMODE:
            return true;

        case N_TEXT:
        case N_SAYTEAM:
            if(ci->chatmute)
            {
                if(!ci->lastchat || totalmillis-ci->lastchat>=5000)
                {
                    ci->lastchat = totalmillis ? totalmillis : 1;
                    sendf(ci->clientnum, 1, "ris", N_SERVMSG, "your text messages are muted");
                }
                return false;
            }
            return true;

        case N_CLIPBOARD:
        case N_EDITF:
        case N_EDITT:
        case N_EDITM:
        case N_FLIP:
        case N_COPY:
        case N_PASTE:
        case N_ROTATE:
        case N_REPLACE:
        case N_DELCUBE:
        case N_REMIP:
        case N_CALCLIGHT:
        case N_EDITENT:
        case N_EDITVAR:
        case N_NEWMAP:
            if(ci->editmute)
            {
                if(!ci->lastedit || totalmillis-ci->lastedit>=10000)
                {
                    ci->lastedit = totalmillis ? totalmillis : 1;
                    sendf(ci->clientnum, 1, "ris", N_SERVMSG, "your edit messages are muted");
                }
                return false;
            }
            return true;

        default: return true;
    }
}

#endif //Z_MSGFILTER_H
