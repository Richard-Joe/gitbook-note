package main

import (
	"flag"
	"fmt"
	"log"
	"os/exec"
	"strings"
	"time"

	"github.com/hashicorp/memberlist"
)

func event2String(e memberlist.NodeEventType) string {
	return []string{"NodeJoin", "NodeLeave", "NodeUpdate"}[e]
}

func main() {
	var mlport int
	var mlname string
	flag.IntVar(&mlport, "mlport", 5555, "memberlist listen port")
	flag.StringVar(&mlname, "mlname", string(rune(mlport)), "memberlist name")
	flag.Parse()

	mconfig := memberlist.DefaultLANConfig()
	mconfig.Name = mlname
	mconfig.BindAddr = "127.0.0.1"
	mconfig.BindPort = mlport
	mconfig.AdvertisePort = mlport

	mlEventCh := make(chan memberlist.NodeEvent, 1024)
	mconfig.Events = &memberlist.ChannelEventDelegate{Ch: mlEventCh}

	ml, err := memberlist.Create(mconfig)
	if err != nil {
		log.Fatalln("failed to create memberlist")
	}

	// memberDiscovery
	go func() {
		// find member from kubernetes cluster
		ticker := time.NewTicker(10 * time.Second)
		for {
			select {
			case <-ticker.C:
				// get members which in group
				members := make(map[string]struct{})
				for _, node := range ml.Members() {
					host := fmt.Sprintf("%s:%d", node.Addr.String(), node.Port)
					members[host] = struct{}{}
				}

				// get members which not in group
				var newMembers []string
				cmd := "netstat -apn | grep memlist | grep LISTEN | awk '{print $4}'"
				output, _ := exec.Command("bash", "-c", cmd).CombinedOutput()
				for _, addr := range strings.Split(string(output), "\n") {
					if len(addr) > 0 {
						if _, isMember := members[addr]; !isMember {
							newMembers = append(newMembers, addr)
						}
					}
				}

				if len(newMembers) > 0 {
					fmt.Printf("find new members: %v\n", len(newMembers))
					// join group
					success, err := ml.Join(newMembers)
					if err != nil || success != len(newMembers) {
						fmt.Errorf("error: %v\n", err)
					} else {
						fmt.Printf("memberlist join succesfully: %v\n", success)
					}
				}
			}
		}
	}()

	ticker := time.NewTicker(30 * time.Second)
	for {
		select {
		case e := <-mlEventCh:
			fmt.Printf("Event: addr (%v), name (%v), event (%v)\n", e.Node.Addr, e.Node.Name, event2String(e.Event))
		case <-ticker.C:
			for _, member := range ml.Members() {
				fmt.Printf("Member: %v\n", *member)
			}
		}
	}

	ml.Shutdown()
}
