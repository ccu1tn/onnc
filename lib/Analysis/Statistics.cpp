//===- Statistics.cpp ------------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <onnc/Analysis/Statistics.h>
#include <onnc/Analysis/StatisticsGroup.h>
#include <onnc/Analysis/GlobalStatistics.h>
#include <onnc/Diagnostic/MsgHandling.h>
#include <onnc/Support/IndentOStream.h>
#include <onnc/JSON/Reader.h>
#include <fstream>

using namespace onnc;

//===----------------------------------------------------------------------===//
// Statistics
//===----------------------------------------------------------------------===//
// Statistics hides the implementation details to keep flexibility for change.
// It is reduced to an adapter of StatisticsPrivate.
//===----------------------------------------------------------------------===//
Statistics::Statistics()
  : m_pGroup(NULL), m_AccessMode(kReadOnly), m_FilePath(), m_Value() {
  // read nothing
}

Statistics::Statistics(StringRef pContent)
  : m_pGroup(NULL), m_AccessMode(kReadOnly), m_FilePath(), m_Value() {
  read(pContent);
}

Statistics::Statistics(const std::string& pContent)
  : m_pGroup(NULL), m_AccessMode(kReadOnly), m_FilePath(), m_Value() {
  read(StringRef(pContent));
}

Statistics::Statistics(const char* pContent)
  : m_pGroup(NULL), m_AccessMode(kReadOnly), m_FilePath(), m_Value() {
  if (NULL == pContent)
    error("can not read NULL json string");
  else
    read(StringRef(pContent));
}

Statistics::Statistics(const Path& pFilePath, AccessMode pMode)
  : m_pGroup(NULL), m_AccessMode(pMode), m_FilePath(pFilePath) {
  open(pFilePath, pMode);
}

Statistics::~Statistics()
{
  sync();
  delete m_pGroup;
}

bool Statistics::isValid() const
{
  return (NULL != m_pGroup);
}

Statistics& Statistics::open(const Path& pFilePath, AccessMode pMode)
{
  if (!isValid()) {
    m_AccessMode = pMode;
    m_FilePath = pFilePath;

    // read the JSON file
    json::Reader reader;
    json::Reader::Result result = reader.parse(m_FilePath, m_Value);
    // can not open file
    if (json::Reader::kCantOpen == result ) {
      error("can not open configuration file:") << m_FilePath.native();
      return *this;
    }

    // illegal format
    if (json::Reader::kIllegal == result ||
        (!m_Value.isObject() && !m_Value.isUndefined())) {
      error("can not parse configuration file:") << m_FilePath.native();
      return *this;
    }

    // empty file
    if (m_Value.isUndefined()) {
      json::Object* object = new json::Object();
      m_Value.delegate(*object);
    }

    // create group
    m_pGroup = new StatisticsGroup(m_Value.asObject());
  }
  return *this;
}

Statistics& Statistics::read(StringRef pContent)
{
  if (pContent.isValid() && !isValid()) {
    m_AccessMode = kReadOnly;

    // reset configuration object
    m_FilePath.clear();
    m_Value.clear();

    // read the JSON content
    json::Reader reader;
    if (!reader.read(pContent, m_Value) || !m_Value.isObject()) {
      error("can not read json string:") <<  pContent;
    }
    else {
      // create group
      m_pGroup = new StatisticsGroup(m_Value.asObject());
    }
  }
  return *this;
}

Statistics::AccessMode Statistics::accessMode() const
{
  return m_AccessMode;
}

StringList Statistics::groupList() const
{
  StringList result;
  m_pGroup->groupList(result);
  return result;
}

void Statistics::groupList(StringList& pGList) const
{
  m_pGroup->groupList(pGList);
}

bool Statistics::hasGroup(StringRef pGroup) const
{
  return m_pGroup->hasGroup(pGroup);
}

bool Statistics::deleteGroup(StringRef pGroup)
{
  return m_pGroup->deleteGroup(pGroup);
}

const StatisticsGroup Statistics::group(StringRef pGroup) const
{
  return m_pGroup->group(pGroup);
}

StatisticsGroup Statistics::group(StringRef pGroup)
{
  return m_pGroup->group(pGroup);
}

StatisticsGroup Statistics::addGroup(StringRef pName)
{
  m_Value.insert(pName, json::Object());
  return this->group(pName);
}

Statistics& Statistics::update(StringRef pName, const StatisticsGroup& pGroup)
{
  m_Value.write(pName, json::Object(*pGroup.m_pObject));
  return *this;
}

Statistics& Statistics::merge(StringRef pName, const StatisticsGroup& pGroup)
{
  m_Value.insert(pName, json::Object(*pGroup.m_pObject));
  return *this;
}

void Statistics::print(std::ostream& pOS) const
{
  IndentOStream ios(pOS, 2);
  m_Value.print(ios);
}

Statistics& Statistics::reset()
{
  delete m_pGroup;
  m_pGroup = NULL;
  m_AccessMode = kReadOnly;
  m_FilePath.clear();
  m_Value.clear();
  return *this;
}

StatisticsGroup Statistics::top()
{
  return StatisticsGroup(m_Value.asObject());
}

bool Statistics::sync()
{
  if (kReadWrite == accessMode() && is_regular(m_FilePath)) {
    std::ofstream ofs(m_FilePath.c_str());
    if (!ofs.is_open())
      return false;

    IndentOStream ios(ofs, 2);
    m_Value.print(ios);
  }
  return true;
}

bool Statistics::addCounter(StringRef pName)
{
  std::string str("no description");
  return addCounter(pName, StringRef(str)); 
}

bool Statistics::addCounter(StringRef pName, StringRef pDesc)
{
  Statistics* gStat = global::stats();
  if (gStat->group("Counter").hasEntry(pName))
    return false;
  gStat->group("Counter").writeEntry(pName, 0);
  gStat->group("Counter_Desc").writeEntry(pName, pDesc);
  return true;
}

bool Statistics::increaseCounter(StringRef pName, unsigned int incNumber)
{
  Statistics* gStat = global::stats();
  if (! gStat->group("Counter").hasEntry(pName))
    return false;
  int entry_value = gStat->group("Counter").readEntry(pName, 0) + incNumber;
  gStat->group("Counter").writeEntry(pName, entry_value);
  return true;
}

bool Statistics::decreaseCounter(StringRef pName, unsigned int decNumber)
{
  Statistics* gStat = global::stats();
  if (! gStat->group("Counter").hasEntry(pName))
    return false;
  int entry_value = gStat->group("Counter").readEntry(pName, 0) - decNumber;
  gStat->group("Counter").writeEntry(pName, entry_value);
  return true;
}

void Statistics::printCounter(StringRef pName, OStream &pOS)
{
  Statistics* gStat = global::stats();
  if (! gStat->group("Counter").hasEntry(pName))
    return;
  pOS << pName << "," 
      << gStat->group("Counter").readEntry(pName, 0) << ","
      << gStat->group("Counter_Desc").readEntry(pName, "no value") 
      // please note that this magic string comes from StatisticsTest.cpp.
      // I guess it's becuase readEntry is implemented by template.
      << std:: endl;
}

StringList Statistics::counterList() const
{
  return global::stats()->group("Counter").entryList();
}

bool Statistics::resetCounter(StringRef pName, int initNum)
{
  Statistics* gStat = global::stats();
  if (! gStat->group("Counter").hasEntry(pName))
    return false;
  gStat->group("Counter").writeEntry(pName, initNum);
  return true;
}
