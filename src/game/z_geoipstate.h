#ifndef Z_GEOIPSTATE_H
#define Z_GEOIPSTATE_H

struct geoipstate
{
    // network identifies which network client is in, like local area network, localhost, anonymous proxy, etc
    char *network, *city, *region, *country, *continent;
    // marked as anonymous network in geoip database
    bool anonymous;
    
    geoipstate(): network(NULL), city(NULL), region(NULL), country(NULL), continent(NULL), anonymous(false) {}
    geoipstate(const geoipstate &s): network(NULL), city(NULL), region(NULL), country(NULL), continent(NULL), anonymous(false) { *this = s; }
    ~geoipstate() { cleanup(); }
    
    void cleanup()
    {
        DELETEP(network);
        DELETEP(city);
        DELETEP(region);
        DELETEP(country);
        DELETEP(continent);
        anonymous = false;
    }
    
    geoipstate &operator =(const geoipstate &s)
    {
        if(&s != this)
        {
            cleanup();
            if(s.network) network = newstring(s.network);
            if(s.city) city = newstring(s.city);
            if(s.region) region = newstring(s.region);
            if(s.country) country = newstring(s.country);
            if(s.continent) continent = newstring(s.continent);
            anonymous = s.anonymous;
        }
        return *this;
    }
};

#endif // Z_GEOIPSTATE_H