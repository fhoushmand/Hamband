#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#ifdef HAMBAND_INFINIBAND
#include "../infiniband/wellcoordination/benchmark/counter-crdt.hpp"
#include "../infiniband/wellcoordination/benchmark/gset-crdt.hpp"
#include "../infiniband/wellcoordination/benchmark/kv-store.hpp"
#include "../infiniband/wellcoordination/benchmark/orset-crdt.hpp"
#include "../infiniband/wellcoordination/benchmark/pnset-crdt.hpp"
#include "../infiniband/wellcoordination/benchmark/register-crdt.hpp"
#include "../infiniband/wellcoordination/benchmark/shop-crdt.hpp"
#include "../infiniband/wellcoordination/benchmark/twopset-crdt.hpp"
#else
#include "../roce/wellcoordination/benchmark/counter-crdt.hpp"
#include "../roce/wellcoordination/benchmark/gset-crdt.hpp"
#include "../roce/wellcoordination/benchmark/kv-store.hpp"
#include "../roce/wellcoordination/benchmark/orset-crdt.hpp"
#include "../roce/wellcoordination/benchmark/pnset-crdt.hpp"
#include "../roce/wellcoordination/benchmark/register-crdt.hpp"
#include "../roce/wellcoordination/benchmark/shop-crdt.hpp"
#include "../roce/wellcoordination/benchmark/twopset-crdt.hpp"
#endif

template <typename Object>
MethodCall issue(Object& object, int method, std::string argument) {
  MethodCall call("test", method, std::move(argument));
  std::string metadata = object.execute(call);
  if (!metadata.empty()) call.arg += "-" + metadata;
  object.executeDownstream(call, false);
  return call;
}

template <typename Object>
void deliver(Object& object, MethodCall const& call) {
  object.executeDownstream(call, true);
}

int main() {
  {
    MethodCall no_argument = ReplicatedObject::createCall("id", "1");
    assert(no_argument.arg.empty());

    Counter counter;
    MethodCall call("id", Counter::ADD, "7");
    std::vector<uint8_t> bytes(counter.serializedSizeCRDT(call));
    size_t written = counter.serializeCRDT(call, bytes.data());
    assert(written == bytes.size());
    uint64_t body_size = 0;
    memcpy(&body_size, bytes.data(), sizeof(body_size));
    assert(body_size + sizeof(body_size) == written);
    MethodCall decoded = counter.deserializeCRDT(bytes.data());
    assert(decoded.id == call.id && decoded.method_type == call.method_type &&
           decoded.arg == call.arg);

    std::string* args = ReplicatedObject::parseArgs("a-b-c", 3);
    assert(args[0] == "a" && args[1] == "b" && args[2] == "c");
    delete[] args;
  }

  {
    Register source_one;
    Register source_two;
    Register first_order;
    Register second_order;
    source_one.setID(1);
    source_two.setID(2);
    MethodCall one = issue(source_one, Register::WRITE, "10");
    MethodCall two = issue(source_two, Register::WRITE, "20");
    deliver(first_order, one);
    deliver(first_order, two);
    deliver(second_order, two);
    deliver(second_order, one);
    MethodCall query("q", Register::QUERY, "");
    assert(first_order.execute(query) == "20");
    assert(second_order.execute(query) == "20");
  }

  {
    KvStore source_one;
    KvStore source_two;
    KvStore first_order;
    KvStore second_order;
    source_one.setID(1);
    source_two.setID(2);
    MethodCall one = issue(source_one, KvStore::PUT, "42-10");
    MethodCall two = issue(source_two, KvStore::PUT, "42-20");
    deliver(first_order, one);
    deliver(first_order, two);
    deliver(second_order, two);
    deliver(second_order, one);
    MethodCall query("q", KvStore::GET, "42");
    assert(first_order.execute(query) == "20");
    assert(second_order.execute(query) == "20");
  }

  {
    ORSet source;
    ORSet receiver;
    source.setID(1);
    MethodCall add = issue(source, ORSet::ADD, "9");
    MethodCall remove = issue(source, ORSet::REMOVE, "9");
    deliver(receiver, remove);
    deliver(receiver, add);
    MethodCall query("q", ORSet::QUERY, "");
    assert(receiver.execute(query) == "0");
  }

  {
    Shop source;
    Shop receiver;
    source.setID(1);
    MethodCall add = issue(source, Shop::ADD, "9-4");
    MethodCall remove = issue(source, Shop::REMOVE, "9");
    deliver(receiver, remove);
    deliver(receiver, add);
    MethodCall query("q", Shop::QUERY, "");
    assert(receiver.execute(query) == "0");
  }

  {
    GSet original;
    original.setsource.insert("local");
    original.setremote.insert("remote");
    GSet copied(original);
    assert(copied.setsource == original.setsource);
    assert(copied.setremote == original.setremote);
  }

  {
    PNSet first;
    PNSet second;
    MethodCall add("a", PNSet::ADD, "7");
    MethodCall remove("r", PNSet::REMOVE, "7");
    deliver(first, add);
    deliver(first, remove);
    deliver(second, remove);
    deliver(second, add);
    assert(first.pnsetremote[7] == second.pnsetremote[7]);
  }

  {
    TWOPSet first;
    TWOPSet second;
    MethodCall add("a", TWOPSet::ADD, "7");
    MethodCall remove("r", TWOPSet::REMOVE, "7");
    deliver(first, add);
    deliver(first, remove);
    deliver(second, remove);
    deliver(second, add);
    assert(first.setremoteadd == second.setremoteadd);
    assert(first.setremoteremove == second.setremoteremove);
  }

  std::cout << "CRDT semantic tests passed" << std::endl;
  return 0;
}
