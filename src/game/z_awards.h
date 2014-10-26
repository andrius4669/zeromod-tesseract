#ifndef Z_AWARDS_H
#define Z_AWARDS_H

VAR(awards, 0, 0, 1);

/*
SVAR(awards_color_scheme, "");

static void z_awards_set_colors(char *colors, size_t len)
{
    size_t i = 0;
    for(; awards_color_scheme[i] && i < len; i++)
        colors[i] = awards_color_scheme[i] >= '0' && awards_color_scheme[i] <= '9' ? awards_color_scheme[i] : '7';
    for(; i < len; i++) colors[i] = i>0 ? colors[i-1] : '7';
}
template<size_t N> static inline void z_awards_set_colors(char (&s)[N]) { z_awards_set_colors(s, N); }
*/

template<typename T>
static T z_awards_best_var_stat(vector<clientinfo *> &best, int offset)
{
    best.setsize(0);
    T bests = T(0);
    loopv(clients)
    {
        if(best.empty())
        {
            best.add(clients[i]);
            bests = *(T *)(((char *)clients[i]) + offset);
        }
        else if(bests == *(T *)(((char *)clients[i]) + offset))
        {
            best.add(clients[i]);
        }
        else if(bests < *(T *)(((char *)clients[i]) + offset))
        {
            best.setsize(0);
            best.add(clients[i]);
            bests = *(T *)(((char *)clients[i]) + offset);
        }
    }
    return bests;
}

template<typename T>
static T z_awards_best_func_stat(vector<clientinfo *> &best, T (* func)(clientinfo *))
{
    best.setsize(0);
    T bests = T(0);
    loopv(clients)
    {
        if(best.empty())
        {
            best.add(clients[i]);
            bests = func(clients[i]);
        }
        else
        {
            T s = func(clients[i]);
            if(s == bests) best.add(clients[i]);
            else if(s > bests)
            {
                best.setsize(0);
                best.add(clients[i]);
                bests = s;
            }
        }
    }
    return bests;
}

static int z_awards_print_best(vector<char> &str, vector<clientinfo *> &best, int max = 0)
{
    int n = 0;
    loopv(best)
    {
        if(n) str.put(", ", 2);
        str.put("\fs\f7", 4);
        const char *name = colorname(best[i]);
        str.put(name, strlen(name));
        str.put("\fr", 2);
        ++n;
        if(max && n >= max) break;
    }
    return n;
}

void z_awards()
{
    const int maxnum = 3;
    vector<char> str;
    vector<clientinfo *> best;
    const char *tp;

    if(!awards || m_edit) return;

    tp = "\f3Awards:\f6";
    str.put(tp, strlen(tp));

    int bestk = z_awards_best_var_stat<int>(best, (((char *)&clients[0]->state.frags) - (char *)clients[0]));
    if(best.length())
    {
        tp = " Kills: ";
        str.put(tp, strlen(tp));
        z_awards_print_best(str, best, maxnum);
        tp = tempformatstring(" \fs\f2(\f6%d\f2)\fr", bestk);
        str.put(tp, strlen(tp));
    }

    double bestp = z_awards_best_func_stat<double>(best, [](clientinfo *ci) { return double(ci->state.frags)/max(ci->state.deaths, 1); });
    if(best.length())
    {
        tp = " KpD: ";
        str.put(tp, strlen(tp));
        z_awards_print_best(str, best, maxnum);
        tp = tempformatstring(" \fs\f2(\f6%.3f\f2)\fr", bestp);
        str.put(tp, strlen(tp));
    }

    int besta = z_awards_best_func_stat<int>(best, [](clientinfo *ci) { return ci->state.damage*100/max(ci->state.shotdamage,1); });
    if(best.length())
    {
        tp = " Acc: ";
        str.put(tp, strlen(tp));
        z_awards_print_best(str, best, maxnum);
        tp = tempformatstring(" \fs\f2(\f6%d\f2)\fr", besta);
        str.put(tp, strlen(tp));
    }

    int bestd = z_awards_best_var_stat<int>(best, (((char *)&clients[0]->state.damage) - (char *)clients[0]));
    if(best.length())
    {
        tp = " Damage: ";
        str.put(tp, strlen(tp));
        z_awards_print_best(str, best, maxnum);
        tp = tempformatstring(" \fs\f2(\f6%d\f2)\fr", bestd);
        str.put(tp, strlen(tp));
    }

    str.add('\0');

    sendservmsg(str.getbuf());
}

#endif // Z_AWARDS_H