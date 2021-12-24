// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "registry_win.h"

#include <stddef.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

// RegEnumValueA() reports the number of characters from the name that were
// written to the buffer, not how many there are. This constant is the maximum
// name size, such that a buffer with this size should read any name.
const DWORD MAX_REGISTRY_NAME_SIZE = 16384;

// Registry values are read as BYTE* but can have char* data whose last
// char is truncated. This function converts the reported |byte_size| to
// a size in char that can store a truncated char if necessary.
inline DWORD to_wchar_size(DWORD byte_size) {
  return (byte_size + sizeof(char) - 1) / sizeof(char);
}

// Mask to pull WOW64 access flags out of REGSAM access.
const REGSAM kWow64AccessMask = KEY_WOW64_32KEY | KEY_WOW64_64KEY;

}  // namespace


// RegKey ----------------------------------------------------------------------

RegKey::RegKey() = default;

RegKey::RegKey(HKEY key) : key_(key) {}

RegKey::RegKey(HKEY rootkey, const char* subkey, REGSAM access) {
  if (rootkey) {
    if (access & (KEY_SET_VALUE | KEY_CREATE_SUB_KEY | KEY_CREATE_LINK))
      Create(rootkey, subkey, access);
    else
      Open(rootkey, subkey, access);
  } else {

    wow64access_ = access & kWow64AccessMask;
  }
}

RegKey::RegKey(RegKey&& other) noexcept
    : key_(other.key_),
      wow64access_(other.wow64access_) {
  other.key_ = nullptr;
  other.wow64access_ = 0;
}

RegKey& RegKey::operator=(RegKey&& other) {
  Close();
  std::swap(key_, other.key_);
  std::swap(wow64access_, other.wow64access_);
  return *this;
}

RegKey::~RegKey() {
  Close();
}

LONG RegKey::Create(HKEY rootkey, const char* subkey, REGSAM access) {
  DWORD disposition_value;
  return CreateWithDisposition(rootkey, subkey, &disposition_value, access);
}

LONG RegKey::CreateWithDisposition(HKEY rootkey,
                                   const char* subkey,
                                   DWORD* disposition,
                                   REGSAM access) {

  HKEY subhkey = nullptr;
  LONG result =
      RegCreateKeyExA(rootkey, subkey, 0, nullptr, REG_OPTION_NON_VOLATILE,
                     access, nullptr, &subhkey, disposition);
  if (result == ERROR_SUCCESS) {
    Close();
    key_ = subhkey;
    wow64access_ = access & kWow64AccessMask;
  }

  return result;
}

LONG RegKey::CreateKey(const char* name, REGSAM access) {

  // After the application has accessed an alternate registry view using one of
  // the [KEY_WOW64_32KEY / KEY_WOW64_64KEY] flags, all subsequent operations
  // (create, delete, or open) on child registry keys must explicitly use the
  // same flag. Otherwise, there can be unexpected behavior.
  // http://msdn.microsoft.com/en-us/library/windows/desktop/aa384129.aspx.
  if ((access & kWow64AccessMask) != wow64access_) {
    return ERROR_INVALID_PARAMETER;
  }
  HKEY subkey = nullptr;
  LONG result = RegCreateKeyExA(key_, name, 0, nullptr, REG_OPTION_NON_VOLATILE,
                               access, nullptr, &subkey, nullptr);
  if (result == ERROR_SUCCESS) {
    Close();
    key_ = subkey;
    wow64access_ = access & kWow64AccessMask;
  }

  return result;
}

LONG RegKey::Open(HKEY rootkey, const char* subkey, REGSAM access) {

  HKEY subhkey = nullptr;

  LONG result = RegOpenKeyExA(rootkey, subkey, 0, access, &subhkey);
  if (result == ERROR_SUCCESS) {
    Close();
    key_ = subhkey;
    wow64access_ = access & kWow64AccessMask;
  }

  return result;
}

LONG RegKey::OpenKey(const char* relative_key_name, REGSAM access) {

  // After the application has accessed an alternate registry view using one of
  // the [KEY_WOW64_32KEY / KEY_WOW64_64KEY] flags, all subsequent operations
  // (create, delete, or open) on child registry keys must explicitly use the
  // same flag. Otherwise, there can be unexpected behavior.
  // http://msdn.microsoft.com/en-us/library/windows/desktop/aa384129.aspx.
  if ((access & kWow64AccessMask) != wow64access_) {
    return ERROR_INVALID_PARAMETER;
  }
  HKEY subkey = nullptr;
  LONG result = RegOpenKeyExA(key_, relative_key_name, 0, access, &subkey);

  // We have to close the current opened key before replacing it with the new
  // one.
  if (result == ERROR_SUCCESS) {
    Close();
    key_ = subkey;
    wow64access_ = access & kWow64AccessMask;
  }
  return result;
}

void RegKey::Close() {
  if (key_) {
    ::RegCloseKey(key_);
    key_ = nullptr;
    wow64access_ = 0;
  }
}

// TODO(wfh): Remove this and other unsafe methods. See http://crbug.com/375400
void RegKey::Set(HKEY key) {
  if (key_ != key) {
    Close();
    key_ = key;
  }
}

HKEY RegKey::Take() {

  HKEY key = key_;
  key_ = nullptr;
  return key;
}

bool RegKey::HasValue(const char* name) const {
  return RegQueryValueExA(key_, name, nullptr, nullptr, nullptr, nullptr) ==
         ERROR_SUCCESS;
}

DWORD RegKey::GetValueCount() const {
  DWORD count = 0;
  LONG result =
      RegQueryInfoKey(key_, nullptr, nullptr, nullptr, nullptr, nullptr,
                      nullptr, &count, nullptr, nullptr, nullptr, nullptr);
  return (result == ERROR_SUCCESS) ? count : 0;
}

LONG RegKey::GetValueNameAt(int index, std::string* name) const {
  char buf[256];
  DWORD bufsize = sizeof(char)*256;
  LONG r = ::RegEnumValueA(key_, index, buf, &bufsize, nullptr, nullptr, nullptr,
                          nullptr);
  if (r == ERROR_SUCCESS)
    name->assign(buf, bufsize);

  return r;
}

LONG RegKey::DeleteKey(const char* name) {

  HKEY subkey = nullptr;

  // Verify the key exists before attempting delete to replicate previous
  // behavior.
  LONG result =
      RegOpenKeyExA(key_, name, 0, READ_CONTROL | wow64access_, &subkey);
  if (result != ERROR_SUCCESS)
    return result;
  RegCloseKey(subkey);

  return RegDelRecurse(key_, name, wow64access_);
}

LONG RegKey::DeleteEmptyKey(const char* name) {

  HKEY target_key = nullptr;
  LONG result =
      RegOpenKeyExA(key_, name, 0, KEY_READ | wow64access_, &target_key);

  if (result != ERROR_SUCCESS)
    return result;

  DWORD count = 0;
  result =
      RegQueryInfoKey(target_key, nullptr, nullptr, nullptr, nullptr, nullptr,
                      nullptr, &count, nullptr, nullptr, nullptr, nullptr);

  RegCloseKey(target_key);

  if (result != ERROR_SUCCESS)
    return result;

  if (count == 0)
    return RegDeleteKeyExA(key_, name, wow64access_, 0);

  return ERROR_DIR_NOT_EMPTY;
}

LONG RegKey::DeleteValue(const char* value_name) {
  LONG result = RegDeleteValueA(key_, value_name);
  return result;
}

LONG RegKey::ReadValueDW(const char* name, DWORD* out_value) const {

  DWORD type = REG_DWORD;
  DWORD size = sizeof(DWORD);
  DWORD local_value = 0;
  LONG result = ReadValue(name, &local_value, &size, &type);
  if (result == ERROR_SUCCESS) {
    if ((type == REG_DWORD || type == REG_BINARY) && size == sizeof(DWORD))
      *out_value = local_value;
    else
      result = ERROR_CANTREAD;
  }

  return result;
}

LONG RegKey::ReadInt64(const char* name, int64_t* out_value) const {

  DWORD type = REG_QWORD;
  int64_t local_value = 0;
  DWORD size = sizeof(local_value);
  LONG result = ReadValue(name, &local_value, &size, &type);
  if (result == ERROR_SUCCESS) {
    if ((type == REG_QWORD || type == REG_BINARY) &&
        size == sizeof(local_value))
      *out_value = local_value;
    else
      result = ERROR_CANTREAD;
  }

  return result;
}

LONG RegKey::ReadValue(const char* name, std::string* out_value) const {

  const size_t kMaxStringLength = 1024;  // This is after expansion.
  // Use the one of the other forms of ReadValue if 1024 is too small for you.
  char raw_value[kMaxStringLength];
  DWORD type = REG_SZ, size = sizeof(raw_value);
  LONG result = ReadValue(name, raw_value, &size, &type);
  if (result == ERROR_SUCCESS) {
    if (type == REG_SZ) {
      *out_value = raw_value;
    } else if (type == REG_EXPAND_SZ) {
      char expanded[kMaxStringLength];
      size = ExpandEnvironmentStringsA(raw_value, expanded, kMaxStringLength);
      // Success: returns the number of char's copied
      // Fail: buffer too small, returns the size required
      // Fail: other, returns 0
      if (size == 0 || size > kMaxStringLength) {
        result = ERROR_MORE_DATA;
      } else {
        *out_value = expanded;
      }
    } else {
      // Not a string. Oops.
      result = ERROR_CANTREAD;
    }
  }

  return result;
}

LONG RegKey::ReadValue(const char* name,
                       void* data,
                       DWORD* dsize,
                       DWORD* dtype) const {
  LONG result = RegQueryValueExA(key_, name, nullptr, dtype,
                                reinterpret_cast<LPBYTE>(data), dsize);
  return result;
}

LONG RegKey::ReadValues(const char* name,
                        std::vector<std::string>* values) {
  values->clear();

  DWORD type = REG_MULTI_SZ;
  DWORD size = 0;
  LONG result = ReadValue(name, nullptr, &size, &type);
  if (result != ERROR_SUCCESS || size == 0)
    return result;

  if (type != REG_MULTI_SZ)
    return ERROR_CANTREAD;

  std::vector<char> buffer(size / sizeof(char));
  result = ReadValue(name, buffer.data(), &size, nullptr);
  if (result != ERROR_SUCCESS || size == 0)
    return result;

  // Parse the double-null-terminated list of strings.
  // Note: This code is paranoid to not read outside of |buf|, in the case where
  // it may not be properly terminated.
  auto entry = buffer.cbegin();
  auto buffer_end = buffer.cend();
  while (entry < buffer_end && *entry != '\0') {
    auto entry_end = std::find(entry, buffer_end, '\0');
    values->emplace_back(entry, entry_end);
    entry = entry_end + 1;
  }
  return 0;
}

LONG RegKey::WriteValue(const char* name, DWORD in_value) {
  return WriteValue(name, &in_value, static_cast<DWORD>(sizeof(in_value)),
                    REG_DWORD);
}

LONG RegKey::WriteValue(const char* name, const char* in_value) {
  return WriteValue(
      name, in_value,
      static_cast<DWORD>(sizeof(*in_value) *
                         (std::char_traits<char>::length(in_value) + 1)),
      REG_SZ);
}

LONG RegKey::WriteValue(const char* name,
                        const void* data,
                        DWORD dsize,
                        DWORD dtype) {
  LONG result =
      RegSetValueExA(key_, name, 0, dtype,
                    reinterpret_cast<LPBYTE>(const_cast<void*>(data)), dsize);
  return result;
}

inline char* WriteInto(std::string* str, size_t length_with_null)
{
    str->reserve(length_with_null);
    str->resize(length_with_null-1);
    return &((*str)[0]);
}

// static
LONG RegKey::RegDelRecurse(HKEY root_key, const char* name, REGSAM access) {
  // First, see if the key can be deleted without having to recurse.
  LONG result = RegDeleteKeyExA(root_key, name, access, 0);
  if (result == ERROR_SUCCESS)
    return result;

  HKEY target_key = nullptr;
  result = RegOpenKeyExA(root_key, name, 0, KEY_ENUMERATE_SUB_KEYS | access,
                        &target_key);

  if (result == ERROR_FILE_NOT_FOUND)
    return ERROR_SUCCESS;
  if (result != ERROR_SUCCESS)
    return result;

  std::string subkey_name(name);

  // Check for an ending slash and add one if it is missing.
  if (!subkey_name.empty() && subkey_name.back() != '\\')
    subkey_name.push_back('\\');

  // Enumerate the keys
  result = ERROR_SUCCESS;
  const DWORD kMaxKeyNameLength = MAX_PATH;
  const size_t base_key_length = subkey_name.length();
  std::string key_name;
  while (result == ERROR_SUCCESS) {
    DWORD key_size = kMaxKeyNameLength;
    result =
        RegEnumKeyExA(target_key, 0, WriteInto(&key_name, kMaxKeyNameLength),
                     &key_size, nullptr, nullptr, nullptr, nullptr);

    if (result != ERROR_SUCCESS)
      break;

    key_name.resize(key_size);
    subkey_name.resize(base_key_length);
    subkey_name += key_name;

    if (RegDelRecurse(root_key, subkey_name.c_str(), access) != ERROR_SUCCESS)
      break;
  }

  RegCloseKey(target_key);

  // Try again to delete the key.
  result = RegDeleteKeyExA(root_key, name, access, 0);

  return result;
}

// RegistryValueIterator ------------------------------------------------------

RegistryValueIterator::RegistryValueIterator(HKEY root_key,
                                             const char* folder_key,
                                             REGSAM wow64access)
    : name_(MAX_PATH, '\0'), value_(MAX_PATH, '\0') {
  Initialize(root_key, folder_key, wow64access);
}

RegistryValueIterator::RegistryValueIterator(HKEY root_key,
                                             const char* folder_key)
    : name_(MAX_PATH, '\0'), value_(MAX_PATH, '\0') {
  Initialize(root_key, folder_key, 0);
}

void RegistryValueIterator::Initialize(HKEY root_key,
                                       const char* folder_key,
                                       REGSAM wow64access) {

  LONG result =
      RegOpenKeyExA(root_key, folder_key, 0, KEY_READ | wow64access, &key_);
  if (result != ERROR_SUCCESS) {
    key_ = nullptr;
  } else {
    DWORD count = 0;
    result =
        ::RegQueryInfoKey(key_, nullptr, nullptr, nullptr, nullptr, nullptr,
                          nullptr, &count, nullptr, nullptr, nullptr, nullptr);

    if (result != ERROR_SUCCESS) {
      ::RegCloseKey(key_);
      key_ = nullptr;
    } else {
      index_ = count - 1;
    }
  }

  Read();
}

RegistryValueIterator::~RegistryValueIterator() {
  if (key_)
    ::RegCloseKey(key_);
}

DWORD RegistryValueIterator::ValueCount() const {
  DWORD count = 0;
  LONG result =
      ::RegQueryInfoKey(key_, nullptr, nullptr, nullptr, nullptr, nullptr,
                        nullptr, &count, nullptr, nullptr, nullptr, nullptr);
  if (result != ERROR_SUCCESS)
    return 0;

  return count;
}

bool RegistryValueIterator::Valid() const {
  return key_ != nullptr && index_ >= 0;
}

void RegistryValueIterator::operator++() {
  --index_;
  Read();
}

bool RegistryValueIterator::Read() {
  if (Valid()) {
    DWORD capacity = static_cast<DWORD>(name_.capacity());
    DWORD name_size = capacity;
    // |value_size_| is in bytes. Reserve the last character for a NUL.
    value_size_ = static_cast<DWORD>((value_.size() - 1) * sizeof(char));
    LONG result = ::RegEnumValueA(
        key_, index_, WriteInto(&name_, name_size), &name_size, nullptr, &type_,
        reinterpret_cast<BYTE*>(value_.data()), &value_size_);

    if (result == ERROR_MORE_DATA) {
      // Registry key names are limited to 255 characters and fit within
      // MAX_PATH (which is 260) but registry value names can use up to 16,383
      // characters and the value itself is not limited
      // (from http://msdn.microsoft.com/en-us/library/windows/desktop/
      // ms724872(v=vs.85).aspx).
      // Resize the buffers and retry if their size caused the failure.
      DWORD value_size_in_wchars = to_wchar_size(value_size_);
      if (value_size_in_wchars + 1 > value_.size())
        value_.resize(value_size_in_wchars + 1, '\0');
      value_size_ = static_cast<DWORD>((value_.size() - 1) * sizeof(char));
      name_size = name_size == capacity ? MAX_REGISTRY_NAME_SIZE : capacity;
      result = ::RegEnumValueA(
          key_, index_, WriteInto(&name_, name_size), &name_size, nullptr,
          &type_, reinterpret_cast<BYTE*>(value_.data()), &value_size_);
    }

    if (result == ERROR_SUCCESS) {
      value_[to_wchar_size(value_size_)] = '\0';
      return true;
    }
  }

  name_[0] = '\0';
  value_[0] = '\0';
  value_size_ = 0;
  return false;
}

// RegistryKeyIterator --------------------------------------------------------

RegistryKeyIterator::RegistryKeyIterator(HKEY root_key,
                                         const char* folder_key) {
  Initialize(root_key, folder_key, 0);
}

RegistryKeyIterator::RegistryKeyIterator(HKEY root_key,
                                         const char* folder_key,
                                         REGSAM wow64access) {
  Initialize(root_key, folder_key, wow64access);
}

RegistryKeyIterator::~RegistryKeyIterator() {
  if (key_)
    ::RegCloseKey(key_);
}

DWORD RegistryKeyIterator::SubkeyCount() const {
  DWORD count = 0;
  LONG result =
      ::RegQueryInfoKey(key_, nullptr, nullptr, nullptr, &count, nullptr,
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  if (result != ERROR_SUCCESS)
    return 0;

  return count;
}

bool RegistryKeyIterator::Valid() const {
  return key_ != nullptr && index_ >= 0;
}

void RegistryKeyIterator::operator++() {
  --index_;
  Read();
}

bool RegistryKeyIterator::Read() {
  if (Valid()) {
    DWORD ncount = static_cast<DWORD>(sizeof(char)*MAX_PATH);
    FILETIME written;
    LONG r = ::RegEnumKeyExA(key_, index_, name_, &ncount, nullptr, nullptr,
                            nullptr, &written);
    if (ERROR_SUCCESS == r)
      return true;
  }

  name_[0] = '\0';
  return false;
}

void RegistryKeyIterator::Initialize(HKEY root_key,
                                     const char* folder_key,
                                     REGSAM wow64access) {
  LONG result =
      RegOpenKeyExA(root_key, folder_key, 0, KEY_READ | wow64access, &key_);
  if (result != ERROR_SUCCESS) {
    key_ = nullptr;
  } else {
    DWORD count = 0;
    result =
        ::RegQueryInfoKey(key_, nullptr, nullptr, nullptr, &count, nullptr,
                          nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    if (result != ERROR_SUCCESS) {
      ::RegCloseKey(key_);
      key_ = nullptr;
    } else {
      index_ = count - 1;
    }
  }

  Read();
}

