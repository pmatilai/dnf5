/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_transaction.hpp"

#include "libdnf/transaction/transaction.hpp"

#include <string>


using namespace libdnf::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionTest);


static Transaction create_transaction(libdnf::Base & base, int nr) {
    auto trans = base.get_transaction_history()->new_transaction();
    trans.set_dt_start(nr * 10 + 1);
    trans.set_dt_end(nr * 10 + 2);
    trans.set_rpmdb_version_begin(libdnf::utils::sformat("ts {} begin", nr));
    trans.set_rpmdb_version_end(libdnf::utils::sformat("ts {} end", nr));
    trans.set_releasever("26");
    trans.set_user_id(1000);
    trans.set_description("dnf install foo");

    return trans;
}


void TransactionTest::test_save_load() {
    auto base = new_base();

    auto trans = create_transaction(*base, 1);
    trans.start();
    trans.finish(TransactionState::OK);

    // load the saved transaction from database and compare values
    auto base2 = new_base();

    auto ts_list = base2->get_transaction_history()->list_transactions({trans.get_id()});
    CPPUNIT_ASSERT_EQUAL(1LU, ts_list.size());

    auto trans2 = ts_list[0];

    CPPUNIT_ASSERT_EQUAL(trans.get_id(), trans2.get_id());
    CPPUNIT_ASSERT_EQUAL(trans.get_dt_start(), trans2.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans.get_dt_end(), trans2.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans.get_rpmdb_version_begin(), trans2.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans.get_rpmdb_version_end(), trans2.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans.get_releasever(), trans2.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans.get_user_id(), trans2.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans.get_description(), trans2.get_description());
    CPPUNIT_ASSERT_EQUAL(trans.get_state(), trans2.get_state());
}


void TransactionTest::test_second_start_raises() {
    auto base = new_base();
    auto trans = base->get_transaction_history()->new_transaction();
    trans.start();
    // 2nd begin must throw an exception
    CPPUNIT_ASSERT_THROW(trans.start(), libdnf::RuntimeError);
}


void TransactionTest::test_save_with_specified_id_raises() {
    auto base = new_base();
    auto trans = base->get_transaction_history()->new_transaction();
    trans.set_id(1);
    // it is not allowed to save a transaction with arbitrary ID
    CPPUNIT_ASSERT_THROW(trans.start(), libdnf::RuntimeError);
}


void TransactionTest::test_update() {
    auto base = new_base();

    auto trans = create_transaction(*base, 1);
    trans.start();

    // modify the values after the initial save
    trans.set_dt_start(12);
    trans.set_dt_end(22);
    trans.set_rpmdb_version_begin("begin_2");
    trans.set_rpmdb_version_end("end_2");
    trans.set_releasever("26_2");
    trans.set_user_id(10002);
    trans.set_description("dnf install foo_2");
    trans.set_state(TransactionState::ERROR);
    trans.finish(TransactionState::OK);

    // load the transction from the database
    auto base2 = new_base();
    auto ts_list = base2->get_transaction_history()->list_transactions({trans.get_id()});
    CPPUNIT_ASSERT_EQUAL(1LU, ts_list.size());

    auto trans2 = ts_list[0];

    // check if the values saved during trans.finish() match
    CPPUNIT_ASSERT_EQUAL(trans.get_id(), trans2.get_id());
    CPPUNIT_ASSERT_EQUAL(trans.get_dt_start(), trans2.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans.get_dt_end(), trans2.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans.get_rpmdb_version_begin(), trans2.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans.get_rpmdb_version_end(), trans2.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans.get_releasever(), trans2.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans.get_user_id(), trans2.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans.get_description(), trans2.get_description());
    CPPUNIT_ASSERT_EQUAL(trans.get_state(), trans2.get_state());
}


void TransactionTest::test_compare() {
    auto base = new_base();

    // test operator ==, > and <
    auto transaction_history = base->get_transaction_history();
    auto first = transaction_history->new_transaction();
    auto second = transaction_history->new_transaction();

    // equal id
    first.set_id(1);
    second.set_id(1);
    CPPUNIT_ASSERT(first == second);

    // compare by id
    second.set_id(2);
    CPPUNIT_ASSERT(first > second);
    CPPUNIT_ASSERT(second < first);

    // equal id and dt_begin
    first.set_id(1);
    second.set_id(1);
    first.set_dt_start(1);
    second.set_dt_start(1);
    CPPUNIT_ASSERT(first == second);
}


void TransactionTest::test_select_all() {
    auto base = new_base();

    auto trans1 = create_transaction(*base, 1);
    trans1.start();
    trans1.finish(TransactionState::OK);

    auto trans2 = create_transaction(*base, 2);
    trans2.start();
    trans2.finish(TransactionState::OK);

    auto trans3 = create_transaction(*base, 3);
    trans3.start();
    trans3.finish(TransactionState::OK);

    // load the saved transaction from database and compare values
    auto ts_list = base->get_transaction_history()->list_all_transactions();
    CPPUNIT_ASSERT_EQUAL(3LU, ts_list.size());

    auto trans1_loaded = ts_list[0];

    CPPUNIT_ASSERT_EQUAL(trans1.get_id(), trans1_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans1.get_dt_start(), trans1_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans1.get_dt_end(), trans1_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans1.get_rpmdb_version_begin(), trans1_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans1.get_rpmdb_version_end(), trans1_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans1.get_releasever(), trans1_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans1.get_user_id(), trans1_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans1.get_description(), trans1_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans1.get_state(), trans1_loaded.get_state());

    auto trans2_loaded = ts_list[1];

    CPPUNIT_ASSERT_EQUAL(trans2.get_id(), trans2_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans2.get_dt_start(), trans2_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans2.get_dt_end(), trans2_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans2.get_rpmdb_version_begin(), trans2_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans2.get_rpmdb_version_end(), trans2_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans2.get_releasever(), trans2_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans2.get_user_id(), trans2_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans2.get_description(), trans2_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans2.get_state(), trans2_loaded.get_state());

    auto trans3_loaded = ts_list[2];

    CPPUNIT_ASSERT_EQUAL(trans3.get_id(), trans3_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans3.get_dt_start(), trans3_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans3.get_dt_end(), trans3_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans3.get_rpmdb_version_begin(), trans3_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans3.get_rpmdb_version_end(), trans3_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans3.get_releasever(), trans3_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans3.get_user_id(), trans3_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans3.get_description(), trans3_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans3.get_state(), trans3_loaded.get_state());
}


void TransactionTest::test_select_multiple() {
    auto base = new_base();

    auto trans1 = create_transaction(*base, 1);
    trans1.start();
    trans1.finish(TransactionState::OK);

    auto trans2 = create_transaction(*base, 2);
    trans2.start();
    trans2.finish(TransactionState::OK);

    auto trans3 = create_transaction(*base, 3);
    trans3.start();
    trans3.finish(TransactionState::OK);

    // load the saved transaction from database and compare values
    auto ts_list = base->get_transaction_history()->list_transactions({1, 3});
    CPPUNIT_ASSERT_EQUAL(2LU, ts_list.size());

    auto trans1_loaded = ts_list[0];

    CPPUNIT_ASSERT_EQUAL(trans1.get_id(), trans1_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans1.get_dt_start(), trans1_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans1.get_dt_end(), trans1_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans1.get_rpmdb_version_begin(), trans1_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans1.get_rpmdb_version_end(), trans1_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans1.get_releasever(), trans1_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans1.get_user_id(), trans1_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans1.get_description(), trans1_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans1.get_state(), trans1_loaded.get_state());

    auto trans3_loaded = ts_list[1];

    CPPUNIT_ASSERT_EQUAL(trans3.get_id(), trans3_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans3.get_dt_start(), trans3_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans3.get_dt_end(), trans3_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans3.get_rpmdb_version_begin(), trans3_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans3.get_rpmdb_version_end(), trans3_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans3.get_releasever(), trans3_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans3.get_user_id(), trans3_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans3.get_description(), trans3_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans3.get_state(), trans3_loaded.get_state());
}


void TransactionTest::test_select_range() {
    auto base = new_base();

    auto trans1 = create_transaction(*base, 1);
    trans1.start();
    trans1.finish(TransactionState::OK);

    auto trans2 = create_transaction(*base, 2);
    trans2.start();
    trans2.finish(TransactionState::OK);

    auto trans3 = create_transaction(*base, 3);
    trans3.start();
    trans3.finish(TransactionState::OK);

    // load the saved transaction from database and compare values
    auto ts_list = base->get_transaction_history()->list_transactions(1, 2);
    CPPUNIT_ASSERT_EQUAL(2LU, ts_list.size());

    auto trans1_loaded = ts_list[0];

    CPPUNIT_ASSERT_EQUAL(trans1.get_id(), trans1_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans1.get_dt_start(), trans1_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans1.get_dt_end(), trans1_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans1.get_rpmdb_version_begin(), trans1_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans1.get_rpmdb_version_end(), trans1_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans1.get_releasever(), trans1_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans1.get_user_id(), trans1_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans1.get_description(), trans1_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans1.get_state(), trans1_loaded.get_state());

    auto trans2_loaded = ts_list[1];

    CPPUNIT_ASSERT_EQUAL(trans2.get_id(), trans2_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans2.get_dt_start(), trans2_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans2.get_dt_end(), trans2_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans2.get_rpmdb_version_begin(), trans2_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans2.get_rpmdb_version_end(), trans2_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans2.get_releasever(), trans2_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans2.get_user_id(), trans2_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans2.get_description(), trans2_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans2.get_state(), trans2_loaded.get_state());
}
