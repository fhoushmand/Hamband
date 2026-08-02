#!/usr/bin/env bash
set -euo pipefail

netdev="${ROCE_NETDEV:-enp135s0f0}"
device="${ROCE_DEVICE:-rxe0}"

sudo modprobe rdma_rxe
sudo ip link set dev "$netdev" up

if ! rdma link show | grep -q "${device}/"; then
  sudo rdma link add "$device" type rxe netdev "$netdev"
fi

rdma link show | grep "${device}/"
ibv_devinfo -d "$device" | grep -E 'hca_id|state:|active_mtu|link_layer'
