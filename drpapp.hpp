
#include <eosio/eosio.hpp>
#include <vector>
#include <string>
#include <map>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include <numeric>

using namespace eosio;
using namespace std;


CONTRACT drpapp : public contract
{
public:

    TABLE cases {
        name claimant_name;
        uint64_t case_id;
        uint64_t number;
        uint8_t stage;
        uint8_t nr_of_requested_arbitrators;
        string case_description;
        map<name, uint8_t> arbitrators;
        vector<string> claims;
        vector<asset> fine;
        vector<asset> relief;
        vector<uint16_t> suspension;
        bool request_ban;
        string claimants_evidence_description;
        vector<string> claimants_ipfs_cids;
        asset claimants_deposit;
        bool claimants_requested_deposit;
        string claimants_telegram;
        string respondents_telegram;
        name respondents_account;
        string other_info_about_respondent;
        bool accusations_accepted_by_respondent;
        string respondents_response;
        vector<string> respondents_ipfs_cids;
        string respondents_evidents_description;
        vector<asset> fine_counter;
        vector<asset> relief_counter;
        vector<uint16_t> suspension_counter;
        asset respondent_deposit;
        bool respondent_requested_deposit;
        name case_winner;
        vector<asset> fine_verdict;
        vector<asset> relief_verdict;
        vector<uint16_t> suspension_verdict;
        string verdict_description;
        map<name, uint8_t> arbitrator_and_signatures;
        vector<string> ipfs_cid_verdict;

        uint64_t primary_key() const { return case_id; }
        uint64_t get_by_number() const { return number; }
    };
    typedef eosio::multi_index<"cases"_n, cases,
    indexed_by<"bynumber"_n, const_mem_fun<cases, uint64_t, &cases::get_by_number>>> cases_t;




    TABLE config {
        name community;
        string community_name;
        string community_description;
        map<uint8_t, string> rec_num_of_arb_and_claim_type;
        uint8_t min_arb_per_case;
        uint8_t max_arb_per_case;
        asset min_deposit;
        uint8_t lead_arb_cut;
        uint8_t other_arb_cut;
        uint32_t time_for_arb_to_accept_the_case;
        uint32_t time_for_respondent_to_acknowledge_the_case;
        uint32_t time_for_respondent_to_respond_the_case;

        uint64_t primary_key() const { return community.value; }
    };
    typedef eosio::multi_index<"config"_n, config> config_t;


    drpapp(name self, name code, datastream<const char *> ds);

    ACTION addcase(name community);
    ACTION addconfig(name community);
  
private:

void require_auth_either(name user1, name user2);


};
