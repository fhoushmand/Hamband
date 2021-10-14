#pragma once 

#include <dory/ctrl/block.hpp>
#include "exchanger.hpp"

namespace dory{
struct ConnectionContext {
  ConnectionContext(ControlBlock &cb, ConnectionExchanger &ce,
                    deleted_unique_ptr<struct ibv_cq> &cq,
                    std::vector<int> &remote_ids, int my_id)
      : cb{cb}, ce{ce}, cq{cq}, remote_ids{remote_ids}, my_id{my_id} {}
  ControlBlock &cb;
  ConnectionExchanger &ce;
  deleted_unique_ptr<struct ibv_cq> &cq;
  std::vector<int> &remote_ids;
  int my_id;
};
}
