#include "tournament.hpp"
#include <algorithm>

tournament::funding_source tournament::funding_source::load(const json& config)
{
    tournament::funding_source ret;

    if(config.has_object("allowed"))
    {
        ret.allowed = config.value<bool>("allowed");
    }

    if(config.has_object("forbid_after_round"))
    {
        ret.forbid_after_round = config.value<size_t>("forbid_after_round");
    }

    if(config.has_object("chips"))
    {
        ret.chips = config.value<size_t>("chips");
    }

    if(config.has_object("cost"))
    {
        ret.cost = config.value<currency>("cost");
    }

    if(config.has_object("commission"))
    {
        ret.commission = config.value<currency>("commission");
    }

    if(config.has_object("equity"))
    {
        ret.equity = config.value<currency>("equity");
    }

    return ret;
}

tournament::player tournament::player::load(const json& config)
{
    tournament::player ret;

    if(config.has_object("name"))
    {
        ret.name = config.value<std::string>("name");
    }
    
    return ret;
}

tournament::chip tournament::chip::load(const json& config)
{
    tournament::chip ret;

    if(config.has_object("color"))
    {
        ret.color = config.value<std::string>("color");
    }

    if(config.has_object("denomination"))
    {
        ret.denomination = config.value<size_t>("denomination");
    }

    if(config.has_object("count_available"))
    {
        ret.count_available = config.value<size_t>("count_available");
    }

    return ret;
}

tournament::blind_level tournament::blind_level::load(const json& config)
{
    tournament::blind_level ret;

    if(config.has_object("little_blind"))
    {
        ret.little_blind = config.value<size_t>("little_blind");
    }

    if(config.has_object("big_blind"))
    {
        ret.big_blind = config.value<size_t>("big_blind");
    }

    if(config.has_object("ante"))
    {
        ret.ante = config.value<size_t>("ante");
    }

    if(config.has_object("duration_ms"))
    {
        ret.duration_ms = config.value<int>("duration_ms");
    }

    if(config.has_object("break_duration_ms"))
    {
        ret.break_duration_ms = config.value<int>("break_duration_ms");
    }
    
    return ret;
}

tournament::seat tournament::seat::load(const json& config)
{
    tournament::seat ret;

    if(config.has_object("player"))
    {
        ret.player = config.value<size_t>("player");
    }

    if(config.has_object("table_number"))
    {
        ret.table_number = config.value<size_t>("table_number");
    }

    if(config.has_object("seat_number"))
    {
        ret.seat_number = config.value<size_t>("seat_number");
    }

    return ret;
}

tournament::payout tournament::payout::load(const json& config)
{
    tournament::payout ret;

    if(config.has_object("derive_award"))
    {
        ret.derive_award = config.value<bool>("derive_award");
    }

    if(config.has_object("percent_x100"))
    {
        ret.percent_x100 = config.value<size_t>("percent_x100");
    }

    if(config.has_object("award"))
    {
        ret.award = config.value<currency>("award");
    }

    return ret;
}

tournament::configuration tournament::configuration::load(const json& config)
{
    tournament::configuration ret;

    if(config.has_object("name"))
    {
        ret.name = config.value<std::string>("name");
    }

    if(config.has_object("cost_currency"))
    {
        ret.cost_currency = config.value<std::string>("cost_currency");
    }

    if(config.has_object("equity_currency"))
    {
        ret.equity_currency = config.value<std::string>("equity_currency");
    }

    if(config.has_object("table_capacity"))
    {
        ret.table_capacity = config.value<size_t>("table_capacity");
    }

    if(config.has_object("buyin"))
    {
        ret.buyin = funding_source::load(config.value<json>("buyin"));
    }

    if(config.has_object("rebuy"))
    {
        ret.rebuy = funding_source::load(config.value<json>("rebuy"));
    }

    if(config.has_object("addon"))
    {
        ret.addon = funding_source::load(config.value<json>("addon"));
    }

    if(config.has_object("players"))
    {
        auto array(config.value<std::vector<json>>("players"));
        std::transform(array.begin(), array.end(), std::back_inserter(ret.players), [](const json& obj){ return player::load(obj); });
    }

    if(config.has_object("chips"))
    {
        auto array(config.value<std::vector<json>>("chips"));
        std::transform(array.begin(), array.end(), std::back_inserter(ret.chips), [](const json& obj){ return chip::load(obj); });
    }

    if(config.has_object("blind_levels"))
    {
        auto array(config.value<std::vector<json>>("blind_levels"));
        std::transform(array.begin(), array.end(), std::back_inserter(ret.blind_levels), [](const json& obj){ return blind_level::load(obj); });
    }

    if(config.has_object("seats"))
    {
        auto array(config.value<std::vector<json>>("seats"));
        std::transform(array.begin(), array.end(), std::back_inserter(ret.seats), [](const json& obj){ return seat::load(obj); });
    }

    if(config.has_object("payouts"))
    {
        auto array(config.value<std::vector<json>>("payouts"));
        std::transform(array.begin(), array.end(), std::back_inserter(ret.payouts), [](const json& obj){ return payout::load(obj); });
    }

    return ret;
}

// construct a tournament, loading configuration from JSON
tournament::tournament(const json& config) : cfg(configuration::load(config))
{
}
