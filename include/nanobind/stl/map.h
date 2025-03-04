#pragma once

#include "detail/nb_dict.h"
#include <map>

NAMESPACE_BEGIN(NB_NAMESPACE)
NAMESPACE_BEGIN(detail)

template <typename Key, typename T, typename Compare, typename Alloc>
struct type_caster<std::map<Key, T, Compare, Alloc>>
 : dict_caster<std::map<Key, T, Compare, Alloc>, Key, T> { };

NAMESPACE_END(detail)
NAMESPACE_END(NB_NAMESPACE)
