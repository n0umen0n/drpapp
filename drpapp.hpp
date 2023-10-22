
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

CONTRACT r4ndomnumb3r : public contract
{
public:
    using contract::contract;

    ACTION generate();
    using generate_action = action_wrapper<"generate"_n, &r4ndomnumb3r::generate>;
};


CONTRACT drpapp : public contract
{
public:

    TABLE rng
    {
        checksum256 value;
    };
    using rng_t = singleton<"rng"_n, rng>;

    TABLE cases 
    {
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
        map<name, string> claimants_socials;
        map<name, string> respondents_socials;
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

    TABLE config 
    {
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

    TABLE communities 
    {
        name community;
        uint64_t primary_key() const { return community.value; }
    };
    typedef eosio::multi_index<"communities"_n, communities> communities_t;

    TABLE arbitrators {
        name arbitrator;
        uint64_t primary_key() const { return arbitrator.value; }
    };
    typedef eosio::multi_index<"arbitrators"_n, arbitrators> arbitrators_t;


    drpapp(name self, name code, datastream<const char *> ds);

    ACTION createcase();
    ACTION addcase(name community);
    ACTION addconfig(name community);
    ACTION delcase(name community,uint64_t case_id);
    ACTION addarbs(name community, vector<name> arbitrator_names);
    ACTION delarbs(name community, vector<name> arbitrator_names);
    ACTION createcase(name community, name claimant_name, uint64_t number, uint8_t nr_of_requested_arbitrators, string case_description, map<name, uint8_t> arbitrators, vector<string> claims, vector<asset> fine, vector<asset> relief, vector<uint16_t> suspension, bool request_ban, string claimants_evidence_description, vector<string> claimants_ipfs_cids, asset claimants_deposit, bool claimants_requested_deposit, map<name, string> claimants_socials, map<name, string> respondents_socials, name respondents_account, string other_info_about_respondent);
    ACTION acceptarbtrn(name arbitrator, uint64_t case_id, name community);
    



    private:

    void require_auth_either(name user1, name user2);

    // simple generic swap
    template<typename T>
    void my_swap(T& t1, T& t2)
    {
        T tmp = move(t1);
        t1 = move(t2);
        t2 = move(tmp);
    }

    // an adaption of: https://cplusplus.com/reference/algorithm/shuffle/
    template<class RandomAccessIterator>
    void my_shuffle(RandomAccessIterator first, RandomAccessIterator last, uint32_t seed)
    {
        for(auto i = (last - first) - 1; i > 0; --i)
        {
            uint32_t r = seed % (i+1);
            my_swap(first[i], first[r]);
            // source: https://stackoverflow.com/a/11946674/2340535
            seed = (seed * 1103515245U + 12345U) & 0x7fffffffU;
        }
    }

    // a simple generic "shell sort" implementation
    // borrowed from: https://www.softwaretestinghelp.com/sorting-techniques-in-cpp/#Shell_Sort
    template <typename T, typename F>
    void my_sort(T arr[], int n, F predicate)
    {
        for(int gap = n/2; gap > 0; gap /= 2)
        {
            for(int i = gap; i < n; i += 1)
            {
                T tmp = arr[i];
                int j;
                for(j = i; j >= gap && predicate(tmp, arr[j - gap]); j -= gap)
                {
                    arr[j] = arr[j - gap];
                }
                arr[j] = tmp;
            }
        }
    }


};
