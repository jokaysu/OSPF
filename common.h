#define NBR_ROUTER 5
#define MY_INF 99999999

struct pkt_HELLO
{
  unsigned int router_id;
  unsigned int link_id;
};

struct pkt_LSPDU
{
  unsigned int sender;
  unsigned int router_id;
  unsigned int link_id;
  unsigned int cost;
  unsigned int via;
};

struct pkt_INIT
{
  unsigned int router_id;
};

struct link_cost
{
  unsigned int link;
  unsigned int cost;
};

struct circuit_DB
{
  unsigned int nbr_link;
  struct link_cost linkcost[NBR_ROUTER];
};
 