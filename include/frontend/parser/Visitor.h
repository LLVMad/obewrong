#ifndef OBW_VISITOR_H
#define OBW_VISITOR_H

// Visitor part
class BaseVisitor
{
public:
  virtual ~BaseVisitor() {}
};

template <class T, typename R = void>
class Visitor
{
public:
  typedef R ReturnType; // Available for clients
  virtual ReturnType visit(T&) = 0;
};

// Visitable part
template <typename R = void>
class BaseVisitable
{
public:
  typedef R ReturnType;
  virtual ~BaseVisitable() {}
  virtual R accept(BaseVisitor&) = 0;
protected:
  template <class T>
  static ReturnType acceptImpl(T& visited, BaseVisitor& guest)
  {
    // Apply the Acyclic Visitor
    if (Visitor<T>* p =
    dynamic_cast<Visitor<T>*>(&guest))
    {
      return p->visit(visited);
    }
    return ReturnType();
  }
};

#define DEFINE_VISITABLE() \
  virtual ReturnType accept(BaseVisitor& guest) \
  { return acceptImpl(*this, guest); }

#endif

