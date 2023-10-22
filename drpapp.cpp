#include "drpapp.hpp"

drpapp::drpapp(
    name self,
    name code,
    datastream<const char *> ds
) : contract(self, code, ds)
{}


void drpapp::createcase(name community, name claimant_name, uint64_t number, uint8_t nr_of_requested_arbitrators,
    string case_description, map<name, uint8_t> arbitrators, vector<string> claims, vector<asset> fine, vector<asset> relief, 
    vector<uint16_t> suspension, bool request_ban, string claimants_evidence_description, vector<string> claimants_ipfs_cids,
    asset claimants_deposit, bool claimants_requested_deposit, map<name, string> claimants_socials, map<name, string> respondents_socials,
    name respondents_account, string other_info_about_respondent)
 {
    require_auth(claimant_name); // Assuming the claimant initiates this action
    check(is_account(respondents_account), "Respondent's account does not exist.");
    // Checks for string lengths
    check(case_description.size() <= 2000, "Case description exceeds 2000 characters.");
    check(claimants_evidence_description.size() <= 2000, "Claimant's evidence description exceeds 2000 characters.");
    check(other_info_about_respondent.size() <= 2000, "Other info about respondent exceeds 2000 characters.");

    // Check the string length for each claim and each IPFS CID
    for(const auto& claim : claims) 
    {
        check(claim.size() <= 400, "A claim description exceeds characters.");
    }

    for(const auto& cid : claimants_ipfs_cids) 
    {
        check(cid.size() <= 46, "A claimant IPFS CID exceeds characters.");
    }

    // Check the string lengths for socials 
    for(const auto& [name, social]: claimants_socials) 
    {
        check(social.size() <= 100, "A claimant's social link exceeds 2000 characters.");
    }

    for(const auto& [name, social]: respondents_socials) 
    {
        check(social.size() <= 100, "A respondent's social link exceeds 2000 characters.");
    }


    action(
        permission_level{get_self(), "active"_n},
        "r4ndomnumb3r"_n,
        "generate"_n,
        std::make_tuple()
    ).send();

    arbitrators_t arbs_table(get_self(), community.value);
    vector<name> all_arbitrators;

    for (auto arb_itr = arbs_table.begin(); arb_itr != arbs_table.end(); ++arb_itr) {
        all_arbitrators.push_back(arb_itr->arbitrator);
    }

    rng_t rndnmbr("r4ndomnumb3r"_n, "r4ndomnumb3r"_n.value);
    checksum256 x = rndnmbr.get().value;
    uint32_t seed = *reinterpret_cast<uint32_t*>(&x);
    my_shuffle(all_arbitrators.begin(), all_arbitrators.end(), seed);

    map<name, uint8_t> selected_arbitrators;
    for (int i = 0; i < nr_of_requested_arbitrators && i < all_arbitrators.size(); i++) 
    {
      selected_arbitrators[all_arbitrators[i]] = 0;
    } 

    cases_t cases_table(get_self(), community.value);
    cases_table.emplace(_self, [&](auto& row) {
        row.case_id = cases_table.available_primary_key();
        row.claimant_name = claimant_name;
        row.number = number;
        row.stage = 1;
        row.nr_of_requested_arbitrators = nr_of_requested_arbitrators;
        row.case_description = case_description;
        row.arbitrators = selected_arbitrators;
        row.claims = claims;
        row.fine = fine;
        row.relief = relief;
        row.suspension = suspension;
        row.request_ban = request_ban;
        row.claimants_evidence_description = claimants_evidence_description;
        row.claimants_ipfs_cids = claimants_ipfs_cids;
        row.claimants_deposit = claimants_deposit;
        row.claimants_requested_deposit = claimants_requested_deposit;
        row.claimants_socials = claimants_socials;
        row.respondents_socials = respondents_socials;
        row.respondents_account = respondents_account;
        row.other_info_about_respondent = other_info_about_respondent;
    });
}

void drpapp::addarbs(name community, vector<name> arbitrator_names) 
{
    require_auth(community);  // Assuming the community itself or a privileged account should do this. Adjust as necessary.

    // Check if the community exists in the config table
    config_t config_table(get_self(), get_self().value);
    auto config_itr = config_table.find(community.value);
    check(config_itr != config_table.end(), "Community does not exist in the config table!");

    arbitrators_t arbs(get_self(), community.value);  // Setting the scope to the community name

    for (const auto& arb_name : arbitrator_names) 
    {
        auto itr = arbs.find(arb_name.value);
        eosio::check(itr == arbs.end(), "Arbitrator already exists!");

        arbs.emplace(get_self(), [&](auto& new_arb) {
            new_arb.arbitrator = arb_name;
        });
    }
}

void drpapp::delarbs(name community, vector<name> arbitrator_names) 
{
    require_auth(community);  // Assuming the community itself or a privileged account should do this. Adjust as necessary.

    arbitrators_t arbs(get_self(), community.value);  // Setting the scope to the community name

    for (const auto& arb_name : arbitrator_names) 
    {
        auto itr = arbs.find(arb_name.value);
        eosio::check(itr != arbs.end(), "Arbitrator not found!");

        arbs.erase(itr);
    }
}

void drpapp::acceptarbtrn(name arbitrator, uint64_t case_id, name community) 
{
    require_auth(arbitrator);  // Assuming the arbitrator themselves should do this. Adjust as necessary.

    cases_t cases_table(get_self(), community.value); // Set the scope to the community name

    // Find the case based on case_id
    auto case_itr = cases_table.find(case_id);
    check(case_itr != cases_table.end(), "Case does not exist!");

    // Check if the arbitrator exists in the map
    auto arb_itr = case_itr->arbitrators.find(arbitrator);
    check(arb_itr != case_itr->arbitrators.end(), "Arbitrator is not associated with this case!");

    // Modify the case to set the uint8_t value for this arbitrator to 1
    cases_table.modify(case_itr, get_self(), [&](auto& row) {
        row.arbitrators[arbitrator] = 1;
    });
}

void drpapp::rejectarbtrn(name arbitrator, uint64_t case_id, name community) 
{
    require_auth(arbitrator);  // Assuming the arbitrator themselves should do this. Adjust as necessary.

    cases_t cases_table(get_self(), community.value); // Set the scope to the community name

    // Find the case based on case_id
    auto case_itr = cases_table.find(case_id);
    check(case_itr != cases_table.end(), "Case does not exist!");

    // Check if the arbitrator exists in the map
    auto arb_itr = case_itr->arbitrators.find(arbitrator);
    check(arb_itr != case_itr->arbitrators.end(), "Arbitrator is not associated with this case!");

    // Fetch all available arbitrators
    arbitrators_t arbs_table(get_self(), community.value);
    vector<name> all_arbitrators;
    for (auto itr = arbs_table.begin(); itr != arbs_table.end(); ++itr) {
        all_arbitrators.push_back(itr->arbitrator);
    }

    // Shuffle to get a random order
    rng_t rndnmbr("r4ndomnumb3r"_n, "r4ndomnumb3r"_n.value);
    checksum256 x = rndnmbr.get().value;
    uint32_t seed = *reinterpret_cast<uint32_t*>(&x);
    my_shuffle(all_arbitrators.begin(), all_arbitrators.end(), seed);

    // Find a new arbitrator that is not in the current arbitrators map
    name new_arbitrator;
    for (auto& arb_name : all_arbitrators) {
        if (case_itr->arbitrators.find(arb_name) == case_itr->arbitrators.end()) {
            new_arbitrator = arb_name;
            break;
        }
    }
    check(is_account(new_arbitrator), "Failed to find a replacement arbitrator.");

    // Modify the case: remove the rejecting arbitrator and add the new arbitrator
    cases_table.modify(case_itr, get_self(), [&](auto& row) {
        row.arbitrators.erase(arbitrator);
        row.arbitrators[new_arbitrator] = 0;
    });
}



void drpapp::addcase(name community) 
{
    cases_t cases(_self, community.value);

    cases.emplace(_self, [&](auto& row) {
        row.claimant_name = "kyotokimura2"_n; 
        row.case_id = cases.available_primary_key();
        row.number = 0;
        row.stage = 1;
        row.nr_of_requested_arbitrators = 1;
        row.case_description = "Default case description.";
        row.arbitrators = {{"vladislav.x"_n, 0}, {"lennyaccount"_n, 1}};
        
        // Adding multiple default values
        row.claims = {"Default claim 1", "Default claim 2", "Default claim 3"};
        row.fine = {asset(10000, symbol("EOS", 4)), asset(9000, symbol("EOS", 4)), asset(8000, symbol("EOS", 4))};
        row.relief = {asset(5000, symbol("EOS", 4)), asset(4000, symbol("EOS", 4)), asset(3000, symbol("EOS", 4))};
        row.suspension = {1, 2, 3};
        
        row.request_ban = false;
        row.claimants_evidence_description = "Default evidence description.";
        row.claimants_ipfs_cids = {"QmDefaultCID1", "QmDefaultCID2"};
        row.claimants_deposit = asset(20000, symbol("EOS", 4));
        row.claimants_requested_deposit = false;

        // Setting the claimants and respondents Telegram account names
        row.claimants_socials = {{"telegram"_n, "@defaultClaimant"}};
        row.respondents_socials = {{"telegram"_n, "@defaultRespondent"}};

        row.respondents_account = "ironscimitar"_n;
        row.other_info_about_respondent = "Default respondent info.";
        row.accusations_accepted_by_respondent = false;
        row.respondents_response = "Default response.";
        row.respondents_ipfs_cids = {"QmDefaultRespCID1"};
        row.respondents_evidents_description = "Default respondent evidence description.";
        row.fine_counter = {asset(500, symbol("EOS", 4)), asset(0, symbol("EOS", 0)), asset(400, symbol("EOS", 4))};
        row.relief_counter = {asset(250, symbol("EOS", 4)), asset(225, symbol("EOS", 0)), asset(200, symbol("EOS", 4))};
        row.suspension_counter = {2, 0, 4};
        row.respondent_deposit = asset(1000, symbol("EOS", 4));
        row.respondent_requested_deposit = false;
        row.case_winner = "defaultwinr"_n;
        row.fine_verdict = {asset(300, symbol("EOS", 4)), asset(290, symbol("EOS", 4)), asset(280, symbol("EOS", 4))};
        row.relief_verdict = {asset(150, symbol("EOS", 4)), asset(145, symbol("EOS", 4)), asset(140, symbol("EOS", 4))};
        row.suspension_verdict = {3, 4, 5};
        row.verdict_description = "Default verdict description.";
        row.arbitrator_and_signatures = {{"vladislav.x"_n, 1}, {"lennyaccount"_n, 0}};
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

void drpapp::delcase(name community, uint64_t case_id) 
{
    cases_t cases(_self, community.value);
    auto itr = cases.find(case_id);

    eosio::check(itr != cases.end(), "Case with specified case_id not found.");

    cases.erase(itr);
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