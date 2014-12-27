#include "gamefunding.hpp"
#include "logger.hpp"
#include <algorithm>
#include <limits>
#include <numeric>

const std::size_t gamefunding::funding_source::always_allow = std::numeric_limits<std::size_t>::max();

// initialize game funding rules
gamefunding::gamefunding() : percent_seats_paid(1.0), total_chips(0), total_cost(0), total_commission(0), total_equity(0)
{
}

// load configuration from JSON (object or file)
void gamefunding::configure(const json& config)
{
    logger(LOG_DEBUG) << "Loading game funding configuration\n";

    config.get_value("cost_currency", this->cost_currency);
    config.get_value("equity_currency", this->equity_currency);
    config.get_value("percent_seats_paid", this->percent_seats_paid);

    std::vector<json> array;
    if(config.get_value("funding_sources", array))
    {
        this->funding_sources.resize(array.size());
        for(std::size_t i(0); i<array.size(); i++)
        {
            array[i].get_value("is_addon", this->funding_sources[i].is_addon);
            array[i].get_value("forbid_after_blind_level", this->funding_sources[i].forbid_after_blind_level);
            array[i].get_value("chips", this->funding_sources[i].chips);
            array[i].get_value("cost", this->funding_sources[i].cost);
            array[i].get_value("commission", this->funding_sources[i].commission);
            array[i].get_value("equity", this->funding_sources[i].equity);
        }
    }
}

// dump configuration to JSON
void gamefunding::dump_configuration(json& config) const
{
    config.set_value("cost_currency", this->cost_currency);
    config.set_value("equity_currency", this->equity_currency);
    config.set_value("percent_seats_paid", this->percent_seats_paid);

    std::vector<json> array;
    for(auto source : this->funding_sources)
    {
        json obj;
        obj.set_value("is_addon", source.is_addon);
        obj.set_value("forbid_after_blind_level", source.forbid_after_blind_level);
        obj.set_value("chips", source.chips);
        obj.set_value("cost", source.cost);
        obj.set_value("commission", source.commission);
        obj.set_value("equity", source.equity);
        array.push_back(obj);
    }
    config.set_value("funding_sources", array);
}

// dump state to JSON
void gamefunding::dump_state(json& state) const
{
    std::vector<player_id> tmp_buyins(this->buyins.begin(), this->buyins.end());
    state.set_value("buyins", json(tmp_buyins));

    std::vector<currency> tmp_payouts(this->payouts.begin(), this->payouts.end());
    state.set_value("payouts", json(tmp_payouts));

    state.set_value("total_chips", this->total_chips);
    state.set_value("total_cost", this->total_cost);
    state.set_value("total_commission", this->total_commission);
    state.set_value("total_equity", this->total_equity);
}

// reset funding information back to zero
void gamefunding::reset()
{
    logger(LOG_DEBUG) << "Resetting all funding information\n";

    this->buyins.clear();
    this->payouts.clear();
    this->total_chips = 0;
    this->total_cost = 0;
    this->total_commission = 0;
    this->total_equity = 0;
}

static constexpr bool operator==(const gamefunding::funding_source& f0, const gamefunding::funding_source& f1)
{
    return f0.is_addon == f1.is_addon &&
           f0.forbid_after_blind_level == f1.forbid_after_blind_level &&
           f0.chips == f1.chips &&
           f0.cost == f1.cost &&
           f0.commission == f1.commission &&
           f0.equity == f1.equity;
}

// fund a player, (re-)buyin or addon
void gamefunding::fund_player(const player_id& player, const funding_source& source, std::size_t current_blind_level)
{
    logger(LOG_DEBUG) << "Funding player " << player << '\n';

    if(current_blind_level > source.forbid_after_blind_level)
    {
        throw tournament_error("too late in the game for this funding source");
    }

    if(source.is_addon && this->buyins.find(player) == this->buyins.end())
    {
        throw tournament_error("tried to addon but not bought in yet");
    }

    if(!source.is_addon && this->buyins.find(player) != this->buyins.end())
    {
        throw tournament_error("player already bought in");
    }

    if(std::find(this->funding_sources.begin(), this->funding_sources.end(), source) == this->funding_sources.end())
    {
        throw tournament_error("funding source not allowed");
    }

    // buy in player
    if(!source.is_addon)
    {
        this->buyins.insert(player);
    }

    // update totals
    this->total_chips += source.chips;
    this->total_cost += source.cost;
    this->total_commission += source.commission;
    this->total_equity += source.equity;
}

// re-calculate payouts
std::vector<gamefunding::currency> gamefunding::recalculate_payouts()
{
    // first, calculate how many places pay, given configuration and number of players bought in
    std::size_t seats_paid(static_cast<std::size_t>(this->buyins.size() * this->percent_seats_paid + 0.5));

    logger(LOG_DEBUG) << "Recalculating payouts: " << seats_paid << " seats will be paid\n";

    // resize our payout structure
    this->payouts.resize(seats_paid);

    // ratio for each seat is comp[seat]:total
    std::vector<double> comp(seats_paid);
    double total(0.0);

    // generate proportional payouts based on harmonic series, 1/N / sum(1/k)
    for(auto n(0); n<seats_paid; n++)
    {
        double c(1.0/(n+1));
        comp[n] = c;
        total += c;
    }

    // next, loop through again generating payouts (rounding fractional payouts down)
    std::transform(comp.begin(), comp.end(), this->payouts.begin(), [&](double c) { return static_cast<currency>(this->total_equity * c / total); });

    // finally, allocating remainder (from rounding) starting from first place
    auto remainder(this->total_equity - std::accumulate(this->payouts.begin(), this->payouts.end(), 0));
    for(auto n(0); n<remainder; n++)
    {
        this->payouts[n]++;
    }

    return this->payouts;
}