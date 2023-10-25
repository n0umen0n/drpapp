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
        row.case_started = current_time_point();
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

void drpapp::joincase(name community, name claimant_name, uint64_t case_id, uint8_t nr_of_requested_arbitrators,
                      string case_description, vector<string> claims, vector<asset> fine, vector<asset> relief,
                      vector<uint16_t> suspension, bool request_ban, string claimants_evidence_description, 
                      vector<string> claimants_ipfs_cids, asset claimants_deposit, bool claimants_requested_deposit, 
                      map<name, string> claimants_socials, map<name, string> respondents_socials, string other_info_about_respondent) 
{
    require_auth(claimant_name);
    check(case_description.size() <= 2000, "Case description exceeds 2000 characters.");
    check(claimants_evidence_description.size() <= 2000, "Claimant's evidence description exceeds 2000 characters.");
    check(other_info_about_respondent.size() <= 2000, "Other info about respondent exceeds 2000 characters.");

    // Other checks (like string lengths for claims, IPFS CIDs, etc.) remain the same as in createcase.

    cases_t cases_table(get_self(), community.value);
    auto old_case_itr = cases_table.find(case_id);
    check(old_case_itr != cases_table.end(), "Old case does not exist!");

    rng_t rndnmbr("r4ndomnumb3r"_n, "r4ndomnumb3r"_n.value);
    checksum256 x = rndnmbr.get().value;
    uint32_t seed = *reinterpret_cast<uint32_t*>(&x);

    map<name, uint8_t> selected_arbitrators = old_case_itr->arbitrators;

    if(nr_of_requested_arbitrators > old_case_itr->nr_of_requested_arbitrators) 
    {
        arbitrators_t arbs_table(get_self(), community.value);
        vector<name> all_arbitrators;

        for (auto arb_itr = arbs_table.begin(); arb_itr != arbs_table.end(); ++arb_itr) 
        {
            all_arbitrators.push_back(arb_itr->arbitrator);
        }

        my_shuffle(all_arbitrators.begin(), all_arbitrators.end(), seed);

        for (int i = 0; i < (nr_of_requested_arbitrators - old_case_itr->nr_of_requested_arbitrators); i++) 
        {
            if(selected_arbitrators.find(all_arbitrators[i]) == selected_arbitrators.end()) 
            {
                selected_arbitrators[all_arbitrators[i]] = 0;
            }
        }
    } 
    else if(nr_of_requested_arbitrators < old_case_itr->nr_of_requested_arbitrators) 
    {
        // Select random arbitrators from the old case
        vector<name> old_arbs;
        for(const auto& [arb_name, value] : old_case_itr->arbitrators) 
        {
            old_arbs.push_back(arb_name);
        }

        my_shuffle(old_arbs.begin(), old_arbs.end(), seed);

        selected_arbitrators.clear();
        for(int i = 0; i < nr_of_requested_arbitrators; i++) 
        {
            selected_arbitrators[old_arbs[i]] = 0;
        }
    }

    cases_table.emplace(_self, [&](auto& row) {
        row.case_id = cases_table.available_primary_key();
        row.case_started = current_time_point();
        row.claimant_name = claimant_name;
        row.number = old_case_itr->number;
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
        row.respondents_account = old_case_itr->respondents_account;
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

//Should be triggered if arbitrators have not accepted nor rejected the case. Certain time period has to pass for this action be triggerable.
void drpapp::swaparb(uint64_t case_id, name community) 
{

    cases_t cases_table(get_self(), community.value); // Set the scope to the community name

    // Find the case based on case_id
    auto case_itr = cases_table.find(case_id);
    check(case_itr != cases_table.end(), "Case does not exist!");

    config_t config_table(get_self(), community.value);
    auto config_itr = config_table.find(community.value);
    check(config_itr != config_table.end(), "Configuration for community not found!");

    time_point_sec current_time = current_time_point();
    time_point_sec deadline = case_itr->case_start_time + seconds(config_itr->time_for_arb_to_accept_the_case);

    if(current_time > deadline) {
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

        // Iterate over case arbitrators and replace the ones that have not accepted
        for(auto& pair : case_itr->arbitrators) {
            if(pair.second == 0) { // Arbitrator did not accept the case
                // Find a new arbitrator that is not in the current arbitrators map
                name new_arbitrator;
                for (auto& arb_name : all_arbitrators) {
                    if (case_itr->arbitrators.find(arb_name) == case_itr->arbitrators.end()) {
                        new_arbitrator = arb_name;
                        break;
                    }
                }
                check(is_account(new_arbitrator), "Failed to find a replacement arbitrator.");

                // Modify the case: replace the arbitrator
                cases_table.modify(case_itr, get_self(), [&](auto& row) {
                    row.arbitrators.erase(pair.first);
                    row.arbitrators[new_arbitrator] = 0;
                    row.case_start_time = current_time_point();
                });
            }
        }
    }
}


void drpapp::acknwdgcase(name respondent, name community, uint64_t case_id) 
{
    require_auth(respondent); // Ensuring that the action is initiated by the respondent.

    // Fetch the cases table with the community as the scope.
    cases_t cases_table(get_self(), community.value);

    // Find the case using the provided case_id.
    auto case_itr = cases_table.find(case_id);
    check(case_itr != cases_table.end(), "Case not found.");
    
    // Ensure that the respondent matches the respondent in the case.
    check(case_itr->respondents_account == respondent, "Account does not match the respondent of the case.");

    // Ensure the case has not already been acknowledged.
    check(!case_itr->case_acknowledged_by_respondent, "Case has already been acknowledged by the respondent.");

    // Modify the case to set the case_acknowledged_by_respondent to true and set the acknowledgment time.
    cases_table.modify(case_itr, get_self(), [&](auto& row) {
        row.case_acknowledged_by_respondent = true;
        row.case_acknowledged_by_respondent_time = current_time_point();
    });
}

void drpapp::giveverdict(
    name lead_arbitrator,
    name community,
    uint64_t case_id,
    vector<asset> fine_verdict,
    vector<asset> relief_verdict,
    vector<uint16_t> suspension_verdict,
    string verdict_description,
    map<name, uint8_t> arbitrator_and_signatures,
    vector<string> ipfs_cid_verdict
) 
{
    // Ensure the action is triggered by the lead arbitrator
    require_auth(lead_arbitrator);

    cases_t cases_table(_self, community.value);
    auto case_itr = cases_table.find(case_id);
    check(case_itr != cases_table.end(), "Case with the given ID doesn't exist.");

    // Ensure that the lead_arbitrator is the first in the arbitrators map
    check(case_itr->arbitrators.begin()->first == lead_arbitrator, "Only the lead arbitrator can give a verdict.");

    // Update the case with the given verdict details
    cases_table.modify(case_itr, _self, [&](auto& row) {
        row.fine_verdict = fine_verdict;
        row.relief_verdict = relief_verdict;
        row.suspension_verdict = suspension_verdict;
        row.verdict_description = verdict_description;
        row.ipfs_cid_verdict = ipfs_cid_verdict;

        // Reset all arbitrator signatures to 0, indicating they have not signed
        for(const auto& [arb_name, _]: case_itr->arbitrators) 
        {
            row.arbitrator_and_signatures[arb_name] = 0;
        }

        // Set the lead arbitrator's signature status to 1
        row.arbitrator_and_signatures[lead_arbitrator] = 1;
    });
}


void drpapp::signverdict(name community, name arbitrator, uint64_t case_id) {
    require_auth(arbitrator);

    cases_t cases_table(_self, community.value);
    auto case_itr = cases_table.find(case_id);
    check(case_itr != cases_table.end(), "Case with the given ID doesn't exist.");

    // Check if the arbitrator exists in the arbitrator_and_signatures map
    auto arb_itr = case_itr->arbitrator_and_signatures.find(arbitrator);
    check(arb_itr != case_itr->arbitrator_and_signatures.end(), "Arbitrator not found in the case.");
    check(arb_itr->second == 0, "Arbitrator has already signed.");

    // Modify the arbitrator's signature status
    cases_table.modify(case_itr, _self, [&](auto& row) {
        row.arbitrator_and_signatures[arbitrator] = 1;
    });

    bool all_arbitrators_signed = true;
    // Check if all arbitrators have signed
    for (const auto& [arb_name, signed_status] : case_itr->arbitrator_and_signatures) {
        if (signed_status == 0) {
            all_arbitrators_signed = false;
            break;
        }
    }

    if (all_arbitrators_signed) {

        int64_t total_claimant_amount = case_itr->claimants_deposit_paid.amount;
        int64_t total_respondent_amount = case_itr->respondent_deposit_paid.amount;

        // Now you can safely perform arithmetic on these int64_t values:
        int64_t total_amount = total_claimant_amount + total_respondent_amount;

        // Construct assets from the resulting amounts:
        asset total_deposit = asset(total_amount, case_itr->claimants_deposit_paid.symbol); 

        // When distributing:
        int64_t lead_arb_share = total_amount * config_itr->lead_arb_cut / 100;
        asset lead_arb_amount = asset(lead_arb_share, case_itr->claimants_deposit_paid.symbol);

        int64_t other_arb_share = (total_amount - lead_arb_share) / (total_arbitrators - 1);
        asset other_arb_amount = asset(other_arb_share, case_itr->claimants_deposit_paid.symbol);

        action(permission_level{_self, "active"_n}, "tethertether"_n, "transfer"_n, make_tuple(_self, case_itr->arbitrators.begin()->first, lead_arb_amount, string("Lead Arbitrator Payout"))).send();
        
        // Remaining arbitrators
        for (auto it = ++(case_itr->arbitrators.begin()); it != case_itr->arbitrators.end(); ++it) {
            action(permission_level{_self, "active"_n}, "tethertether"_n, "transfer"_n, make_tuple(_self, it->first, other_arb_amount, string("Arbitrator Payout"))).send();
        }

        // Update the case stage
        cases_table.modify(case_itr, _self, [&](auto& row) {
            row.stage = 5;
        });


        }
}


void drpapp::addcomm(
    name community,
    string community_name,
    string community_description,
    map<uint8_t, string> rec_num_of_arb_and_claim_type,
    uint8_t min_arb_per_case,
    uint8_t max_arb_per_case,
    asset min_deposit,
    uint8_t lead_arb_cut,
    uint8_t other_arb_cut,
    uint32_t time_for_arb_to_accept_the_case,
    uint32_t time_for_respondent_to_acknowledge_the_case,
    uint32_t time_for_respondent_to_respond_the_case) 
{

    // Initialize the config table with the scope of community name
    config_t config_table(get_self(), _self.value);

    // Check if the community configuration already exists
    auto existing_config = config_table.find(community.value);
    check(existing_config == config_table.end(), "Community configuration already exists!");

    // Add to the table
    config_table.emplace(get_self(), [&](auto& row) {
        row.community = community;
        row.community_name = community_name;
        row.community_description = community_description;
        row.rec_num_of_arb_and_claim_type = rec_num_of_arb_and_claim_type;
        row.min_arb_per_case = min_arb_per_case;
        row.max_arb_per_case = max_arb_per_case;
        row.min_deposit = min_deposit;
        row.lead_arb_cut = lead_arb_cut;
        row.other_arb_cut = other_arb_cut;
        row.time_for_arb_to_accept_the_case = time_for_arb_to_accept_the_case;
        row.time_for_respondent_to_acknowledge_the_case = time_for_respondent_to_acknowledge_the_case;
        row.time_for_respondent_to_respond_the_case = time_for_respondent_to_respond_the_case;
    });
}

void drpapp::assetin(
    name from,
    name to,
    asset quantity,
    string memo
)
{
    if(to == _self)
    {
        // Split the memo by ',' to extract case_id, depositor type, and community name
        auto first_comma = memo.find(',');
        check(first_comma != string::npos && first_comma < memo.size() - 1, "Invalid memo format.");

        auto second_comma = memo.find(',', first_comma + 1);
        check(second_comma != string::npos && second_comma < memo.size() - 1, "Invalid memo format.");

        uint64_t case_id_val = std::stoull(memo.substr(0, first_comma));
        string depositor_type = memo.substr(first_comma + 1, second_comma - first_comma - 1);
        name community_name = name(memo.substr(second_comma + 1));

        cases_t cases_table(_self, community_name.value);
        auto case_itr = cases_table.find(case_id_val);
        check(case_itr != cases_table.end(), "Case with the given ID doesn't exist.");

        if(depositor_type == "claimant") 
        {
            cases_table.modify(case_itr, _self, [&](auto& row) {
                row.claimants_deposit_paid += quantity;
            });
        } 
        else if(depositor_type == "respondent")
        {
            cases_table.modify(case_itr, _self, [&](auto& row) {
                row.respondent_deposit_paid += quantity;
            });
        } 
        else 
        {
            check(false, "Invalid depositor type in memo.");
        }
    }
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