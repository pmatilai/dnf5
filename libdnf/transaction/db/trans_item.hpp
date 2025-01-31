/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_TRANSACTION_DB_TRANS_ITEM_HPP
#define LIBDNF_TRANSACTION_DB_TRANS_ITEM_HPP


#include "utils/sqlite3/sqlite3.hpp"

#include <memory>


namespace libdnf::transaction {


class TransactionItem;


/// Copy 'trans_item' fields from a query to TransactionItem or an object that inherits from it
void transaction_item_select(libdnf::utils::SQLite3::Query & query, TransactionItem & ti);


/// Create a query (statement) that inserts new records to the 'trans_item' table
std::unique_ptr<libdnf::utils::SQLite3::Statement> trans_item_insert_new_query(libdnf::utils::SQLite3 & conn);


/// Use a query to insert a new record to the 'trans_item' table
int64_t transaction_item_insert(libdnf::utils::SQLite3::Statement & query, TransactionItem & ti);


}  // namespace libdnf::transaction


#endif  // LIBDNF_TRANSACTION_DB_TRANS_ITEM_HPP
