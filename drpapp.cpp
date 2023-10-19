#include "drpapp.hpp"

drpapp::drpapp(
    name self,
    name code,
    datastream<const char *> ds
) : contract(self, code, ds)
{}

void drpapp::addcase(name community) 
{
cases_t cases(_self, community.value);

    cases.emplace(_self, [&](auto& row) {
        row.claimant_name = "deftclaimnt"_n; 
        row.case_id = cases.available_primary_key();
        row.number = 0;
        row.stage = 1;
        row.nr_of_requested_arbitrators = 1;
        row.case_description = "Default case description.";
        row.arbitrators = {{"arbitrator1"_n, 0}, {"arbitrator2"_n, 1}};
        row.claims = {"Default claim"};
        row.fine = {asset(10000, symbol("EOS", 4))};  // 1.0000 EOS as default fine
        row.relief = {asset(5000, symbol("EOS", 4))}; // 0.5000 EOS as default relief
        row.suspension = {1};
        row.request_ban = false;
        row.claimants_evidence_description = "Default evidence description.";
        row.claimants_ipfs_cids = {"QmDefaultCID1", "QmDefaultCID2"};
        row.claimants_deposit = asset(20000, symbol("EOS", 4)); // 2.0000 EOS
        row.claimants_requested_deposit = false;
        row.claimants_telegram = "@defaultClaimant";
        row.respondents_telegram = "@defaultRespondent";
        row.respondents_account = "defaultresp"_n;
        row.other_info_about_respondent = "Default respondent info.";
        row.accusations_accepted_by_respondent = false;
        row.respondents_response = "Default response.";
        row.respondents_ipfs_cids = {"QmDefaultRespCID1"};
        row.respondents_evidents_description = "Default respondent evidence description.";
        row.fine_counter = {asset(500, symbol("EOS", 4))}; // 0.0500 EOS
        row.relief_counter = {asset(250, symbol("EOS", 4))}; // 0.0250 EOS
        row.suspension_counter = {2};
        row.respondent_deposit = asset(1000, symbol("EOS", 4)); // 0.1000 EOS
        row.respondent_requested_deposit = false;
        row.case_winner = "defaultwinr"_n;
        row.fine_verdict = {asset(300, symbol("EOS", 4))}; // 0.0300 EOS
        row.relief_verdict = {asset(150, symbol("EOS", 4))}; // 0.0150 EOS
        row.suspension_verdict = {3};
        row.verdict_description = "Default verdict description.";
        row.arbitrator_and_signatures = {{"arbitrator1"_n, 1}, {"arbitrator2"_n, 0}};
        row.ipfs_cid_verdict = {"QmVerdictCID"};
    });
}

void drpapp::addconfig(name community) 
{

    config_t configs(_self, community.value);

    configs.emplace(_self, [&](auto& row) {
        row.community = "defaultcomm"_n;
        row.community_name = "Default Community Name";
        row.community_description = "This is a default community description.";
        row.rec_num_of_arb_and_claim_type = {{1, "type1"}, {2, "type2"}}; // Default record number and claim types
        row.min_arb_per_case = 3; // Default minimum arbitrators per case
        row.max_arb_per_case = 3; // Default maximum arbitrators per case
        row.min_deposit = asset(1000, symbol("EOS", 4)); // Default minimum deposit of 0.1000 EOS
        row.lead_arb_cut = 50;  // Default lead arbitrator cut as 50% (0.5)
        row.other_arb_cut = 50; // Default other arbitrator cut as 25% (0.25)
        row.time_for_arb_to_accept_the_case = 86400; // Default time for an arbitrator to accept the case: 24 hours (in seconds)
        row.time_for_respondent_to_acknowledge_the_case = 86400; // 24 hours (in seconds)
        row.time_for_respondent_to_respond_the_case = 172800; // 48 hours (in seconds)
    });

}


void drpapp::require_auth_either(name user1, name user2) {
  if (has_auth(user1)) {
    require_auth(user1);
  } else if (has_auth(user2)) {
    require_auth(user2);
  } else {
    check(false, "Missing required authority");
  }
}