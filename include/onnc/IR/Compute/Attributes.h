//===- Attributes.h -------------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef ONNC_IR_COMPUTE_ATTRIBUTES_H
#define ONNC_IR_COMPUTE_ATTRIBUTES_H
#include <onnc/IR/Compute/Tensor.h>
#include <onnc/IR/ComputeGraph.h>
#include <string>

namespace onnc {

class Attribute
{
public:
  enum Type {
    kFloat,
    kInteger,
    kString,
    kTensor,
    kGraph,
    kFloatVec,
    kIntegerVec,
    kStringVec,
    kTensorVec,
    kGraphVec
  };

public:
  Attribute(Type pKind)
    : m_Kind(pKind) {
  }

  Attribute(const Attribute& pCopy)
    : m_Kind(pCopy.kind()) {
  }

  virtual ~Attribute() { }

  Attribute& operator=(const Attribute& pCopy) {
    m_Kind = pCopy.kind();
    return *this;
  }

  Type kind() const { return m_Kind; }

private:
  Type m_Kind;
};

template<typename ValueType, Attribute::Type Kind>
class ScalarAttribute : public Attribute
{
public:
  ScalarAttribute()
    : Attribute(Kind), m_Value() {
  }

  ScalarAttribute(const ValueType& pValue)
    : Attribute(Kind), m_Value(pValue) {
  }

  ScalarAttribute(const ScalarAttribute& pCopy)
    : Attribute(pCopy), m_Value(pCopy.value()) {
  }

  virtual ~ScalarAttribute() { }

  ScalarAttribute& operator=(const ScalarAttribute& pCopy) {
    Attribute::operator=(pCopy);
    m_Value = pCopy.value();
    return *this;
  }

  /// return a copy
  ValueType value() { return m_Value; }

  const ValueType& value() const { return m_Value; }

  void setValue(const ValueType& pValue) { m_Value = pValue; }

private:
  ValueType m_Value;
};

template<typename ValueType, Attribute::Type Kind>
class VectorAttribute : public Attribute
{
public:
  typedef std::vector<ValueType> VectorType;

public:
  VectorAttribute()
    : Attribute(Kind), m_Vector() {
  }

  VectorAttribute(const VectorType& pVector)
    : Attribute(Kind), m_Vector(pVector) {
  }

  VectorAttribute(VectorType&& pVector)
    : Attribute(Kind), m_Vector(pVector) {
  }

  VectorAttribute(const VectorAttribute& pCopy)
    : Attribute(pCopy), m_Vector(pCopy.vector()) {
  }

  virtual ~VectorAttribute() { }

  VectorAttribute& operator=(const VectorAttribute& pCopy) {
    Attribute::operator=(pCopy);
    m_Vector = pCopy.vector();
    return *this;
  }

  VectorType& vector() { return m_Vector; }

  const VectorType& vector() const { return m_Vector; }

private:
  VectorType m_Vector;
};

typedef ScalarAttribute<double, Attribute::kFloat>       FloatAttr;
typedef ScalarAttribute<int64_t, Attribute::kInteger>    IntAttr;
typedef ScalarAttribute<std::string, Attribute::kString> StringAttr;
typedef ScalarAttribute<Tensor, Attribute::kTensor>      TensorAttr;
typedef ScalarAttribute<ComputeGraph, Attribute::kGraph> GraphAttr;

typedef VectorAttribute<double, Attribute::kFloat>       FloatsAttr;
typedef VectorAttribute<int64_t, Attribute::kInteger>    IntsAttr;
typedef VectorAttribute<std::string, Attribute::kString> StringsAttr;
typedef VectorAttribute<Tensor, Attribute::kTensor>      TensorsAttr;
typedef VectorAttribute<ComputeGraph, Attribute::kGraph> GraphsAttr;

} // namespace of onnc

#endif