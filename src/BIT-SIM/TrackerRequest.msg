packet TrackerRequest
{
    string peer_id;
    int uploaded;
    int downloaded;
    int left;

    short numwant;
    string event;
}
