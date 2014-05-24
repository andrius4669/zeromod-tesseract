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
            if(cq && cq->chatmute)
            {
                if(!cq->lastchat || totalmillis-cq->lastchat>=2000)
                {
                    cq->lastchat = totalmillis ? totalmillis : 1;
                    if(cq->state.aitype == AI_NONE) sendf(cq->clientnum, 1, "ris", N_SERVMSG, "your text messages are muted");
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
        case N_EDITENT:
        case N_EDITVAR:
            if(ci->editmute)
            {
                if(!ci->lastedit || totalmillis-ci->lastedit>=5000)
                {
                    ci->lastedit = totalmillis ? totalmillis : 1;
                    sendf(ci->clientnum, 1, "ris", N_SERVMSG, "your edit message was muted");
                }
                return false;
            }
            return true;
        case N_REMIP:
            if(ci->editmute) { sendf(ci->clientnum, 1, "ris", N_SERVMSG, "your remip was muted"); return false; }
            return true;
        case N_CALCLIGHT:
            if(ci->editmute) { sendf(ci->clientnum, 1, "ris", N_SERVMSG, "your calclight was muted"); return false; }
            return true;
        case N_NEWMAP:
            if(ci->editmute) { sendf(ci->clientnum, 1, "ris", N_SERVMSG, "your newmap was muted"); return false; }
            return true;
        default: return true;
    }
}

#endif //Z_MSGFILTER_H
