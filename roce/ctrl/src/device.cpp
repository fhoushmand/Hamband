#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include "device.hpp"

// OpenDevice definitions
namespace dory {
OpenDevice::OpenDevice() { memset(&device_attr, 0, sizeof(device_attr)); }

OpenDevice::OpenDevice(struct ibv_device *device) : dev{device} {
  ctx = ibv_open_device(device);
  if (ctx == nullptr) {
    throw std::runtime_error("Could not get device list: " +
                             std::string(std::strerror(errno)));
  }

  memset(&device_attr, 0, sizeof(device_attr));
  if (ibv_query_device(ctx, &device_attr) != 0) {
    throw std::runtime_error("Could not query device: " +
                             std::string(std::strerror(errno)));
  }
}

OpenDevice::~OpenDevice() {
  if (ctx != nullptr) {
    ibv_close_device(ctx);
  }
}

// Copy constructor
OpenDevice::OpenDevice(OpenDevice const &o) : dev{o.dev} {
  ctx = ibv_open_device(dev);
  if (ctx == nullptr) {
    throw std::runtime_error("Could not get device list: " +
                             std::string(std::strerror(errno)));
  }

  memset(&device_attr, 0, sizeof(device_attr));
  if (ibv_query_device(ctx, &device_attr) != 0) {
    throw std::runtime_error("Could not query device: " +
                             std::string(std::strerror(errno)));
  }
}

// Move constructor
OpenDevice::OpenDevice(OpenDevice &&o)
    : dev{o.dev}, ctx{o.ctx}, device_attr(o.device_attr) {
  o.ctx = nullptr;
}

// Copy assignment operator
OpenDevice &OpenDevice::operator=(OpenDevice const &o) {
  if (&o == this) {
    return *this;
  }

  ctx = ibv_open_device(o.dev);
  if (ctx == nullptr) {
    throw std::runtime_error("Could not get device list: " +
                             std::string(std::strerror(errno)));
  }

  memset(&device_attr, 0, sizeof(device_attr));
  if (ibv_query_device(ctx, &device_attr) != 0) {
    throw std::runtime_error("Could not query device: " +
                             std::string(std::strerror(errno)));
  }

  return *this;
}

// Move assignment operator
OpenDevice &OpenDevice::operator=(OpenDevice &&o) {
  if (&o == this) {
    return *this;
  }

  dev = o.dev;
  ctx = o.ctx;
  device_attr = o.device_attr;
  o.ctx = nullptr;

  return *this;
}

struct ibv_device_attr const &OpenDevice::device_attributes() const {
  return device_attr;
}
}  // namespace dory

// Device definitions
namespace dory {
Devices::Devices() : dev_list{nullptr} {}

Devices::~Devices() {
  if (dev_list != nullptr) {
    ibv_free_device_list(dev_list);
  }
}

std::vector<OpenDevice> &Devices::list(bool force) {
  if (force || dev_list == nullptr) {
    int num_devices = 0;
    dev_list = ibv_get_device_list(&num_devices);

    if (dev_list == nullptr) {
      throw std::runtime_error("Error getting device list: " +
                               std::string(std::strerror(errno)));
    }

    for (int i = 0; i < num_devices; i++) {
      devices.push_back(OpenDevice(dev_list[i]));
    }
  }

  return devices;
}

OpenDevice Devices::select() {
  auto &available = list();
  if (available.empty()) {
    throw std::runtime_error("No RDMA devices are available");
  }

  auto const *requested = std::getenv("DORY_RDMA_DEVICE");
  if (requested == nullptr || requested[0] == '\0') {
    return std::move(available.back());
  }

  for (auto &device : available) {
    if (std::strcmp(device.name(), requested) == 0) {
      return std::move(device);
    }
  }

  throw std::runtime_error("Requested RDMA device '" +
                           std::string(requested) + "' was not found");
}
}  // namespace dory

namespace dory {
ResolvedPort::ResolvedPort(OpenDevice &od)
    : open_dev{od},
      port_index{-1},
      port_id{0},
      port_lid{0},
      link_layer{IBV_LINK_LAYER_UNSPECIFIED},
      gid_index{0},
      active_mtu{IBV_MTU_256} {
  (void)port_index;
  memset(&port_gid, 0, sizeof(port_gid));
}

bool ResolvedPort::bindTo(size_t index) {
  size_t skipped_active_ports = 0;
  for (uint8_t i = 1; i <= open_dev.device_attributes().phys_port_cnt; i++) {
    struct ibv_port_attr port_attr;
    memset(&port_attr, 0, sizeof(ibv_port_attr));

    if (ibv_query_port(open_dev.context(), i, &port_attr)) {
      throw std::runtime_error("Failed to query port: " +
                               std::string(std::strerror(errno)));
    }

    if (port_attr.phys_state != IBV_PORT_ACTIVE &&
        port_attr.phys_state != IBV_PORT_ACTIVE_DEFER) {
      continue;
    }

    if (skipped_active_ports == index) {
      if (port_attr.link_layer != IBV_LINK_LAYER_INFINIBAND &&
          port_attr.link_layer != IBV_LINK_LAYER_ETHERNET) {
        throw std::runtime_error(
            "Unsupported RDMA port link layer " +
            link_layer_str(port_attr.link_layer));
      }

      port_id = i;
      port_lid = port_attr.lid;
      link_layer = port_attr.link_layer;
      active_mtu = port_attr.active_mtu;

      if (link_layer == IBV_LINK_LAYER_ETHERNET) {
        auto const *configured_gid = std::getenv("DORY_GID_INDEX");
        if (configured_gid != nullptr && configured_gid[0] != '\0') {
          char *end = nullptr;
          errno = 0;
          auto parsed = std::strtol(configured_gid, &end, 10);
          if (errno != 0 || end == configured_gid || *end != '\0' ||
              parsed < 0 || parsed > 255) {
            throw std::runtime_error("DORY_GID_INDEX must be between 0 and 255");
          }
          gid_index = static_cast<uint8_t>(parsed);
        }

        if (ibv_query_gid(open_dev.context(), port_id, gid_index, &port_gid) !=
            0) {
          throw std::runtime_error("Failed to query RoCE GID: " +
                                   std::string(std::strerror(errno)));
        }
      }

      return true;
    }

    skipped_active_ports += 1;
  }

  return false;
}
}  // namespace dory
