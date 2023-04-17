// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "eva/ir/attributes.h"
#include "eva/ir/ops.h"
#include "eva/ir/types.h"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <ostream>
#include <unordered_set>
#include <vector>

namespace eva {

class Program;

class Term : public AttributeList, public std::enable_shared_from_this<Term> {
public:
  using Ptr = std::shared_ptr<Term>;  //对外隐藏类型, 比如要遍历operands, 遍历器类型直接可以用Ptr

  Term(Op opcode, Program &program);  //构造函数，指明这个节点的类型和他所在的DAG图
  ~Term();

  void addOperand(const Ptr &term);
  bool eraseOperand(const Ptr &term);
  bool replaceOperand(Ptr oldTerm, Ptr newTerm); //替换当前节点引用的节点，从oldTerm替换到newTerm，细节见文档

  void setOperands(std::vector<Ptr> o);
  std::size_t numOperands() const;
  Ptr operandAt(size_t i); //按下标索引u-d链中的值
  const std::vector<Ptr> &getOperands() const;

  void replaceUsesWithIf(Ptr term, std::function<bool(const Ptr &)>);
  void replaceAllUsesWith(Ptr term);
  void replaceOtherUsesWith(Ptr term); 

  /*
  replace*UsesWith* 负责在编译DAG时使用，当某个地方需要优化时可以直接修改，他的底层是调用上面的replaceOperand实现的
  */

  std::size_t numUses();
  std::vector<Ptr> getUses(); //这里将uses表中的每个 Term* 转为Ptr类型保存在表中返回给外部使用，因为每个节点又增加了外部引用，所以要用智能指针，免得在其他地方被释放导致这里的引用为空或是在这里的外部引用产生内存泄漏

  bool isInternal() const; //返回该节点在DAG图中是否是中间节点(相对于根节点和叶节点)

  const Op op;  //当前节点的运算符类型
  Program &program; //当前的节点保存属于哪个DAG图

  // Unique index for this Term in the owning Program. Managed by Program
  // and used to index into TermMap instances.
  std::uint64_t index;

  friend std::ostream &operator<<(std::ostream &s, const Term &term);

private:
  std::vector<Ptr> operands; // use->def chain (unmanaged pointers)  这里使用Ptr是为了保证这个节点(Term对象)所指向的它使用的那个定义节点不会被释放掉
  std::vector<Term *> uses;  // def->use chain (managed pointers)  这里直接使用Term是因为如果使用该对象的运算符节点全都被释放了，那么指向该对象的智能指针引用计数为0，该对象就不存在了

  void addUse(Term *term);  //有新的节点要使用当前节点，对当前节点增加一个使用条目
  bool eraseUse(Term *term); //与上条相反
};

} // namespace eva
