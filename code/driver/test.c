#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/netfilter.h>
#include <net/net_namespace.h>
#include <net/protocol.h>

MODULE_LICENSE("Dual BSD/GPL");

static void cur_process(void) {
    printk(KERN_ALERT "The process is \"%s\" (pid %i)\n",
        current->comm, current->pid);
}

char id2str[NF_INET_NUMHOOKS][30] = {
    "PRE_ROUTING",
    "LOCAL_IN",
    "FORWARD",
    "LOCAL_OUT",
    "POST_ROUTING",
};

static void get_all_ipv4_netfilter_hooks(void) {
    struct net *net = &init_net;
    unsigned int hooknum;
    struct nf_hook_entries *p;
    int i;

    printk("net->ifindex: %d\n", net->ifindex);
    for (hooknum = NF_INET_PRE_ROUTING; hooknum < NF_INET_NUMHOOKS; hooknum++) {
        printk("==== %s ====\n", id2str[hooknum]);
        p = *(net->nf.hooks_ipv4 + hooknum);
        if (p == NULL) {
            continue;
        }
        for (i = 0; i < p->num_hook_entries; i++) {
            printk("        %pS\n", p->hooks[i].hook);
        }
        printk("\n");
    }
}

static int test_init(void) {
    printk(KERN_ALERT "Hello, Test init\n");
    cur_process();
    get_all_ipv4_netfilter_hooks();
    return 0;
}

static void test_exit(void) {
    printk(KERN_ALERT "Googbye, Test\n");
}

module_init(test_init);
module_exit(test_exit);