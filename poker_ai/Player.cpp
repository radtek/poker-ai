#include "Player.hpp"
#include <algorithm>
#include <map>
#include <unordered_map>
#include "Util.hpp"

static const unsigned int possibleHandCombinations[21][5]  = {
	{1, 2, 3, 4, 5},
	{1, 2, 3, 4, 6},
	{1, 2, 3, 4, 7},
	{1, 2, 3, 5, 6},
	{1, 2, 3, 5, 7},
	{1, 2, 3, 6, 7},
	{1, 2, 4, 5, 6},
	{1, 2, 4, 5, 7},
	{1, 2, 4, 6, 7},
	{1, 2, 5, 6, 7},
	{1, 3, 4, 5, 6},
	{1, 3, 4, 5, 7},
	{1, 3, 4, 6, 7},
	{1, 3, 5, 6, 7},
	{1, 4, 5, 6, 7},
	{2, 3, 4, 5, 6},
	{2, 3, 4, 5, 7},
	{2, 3, 4, 6, 7},
	{2, 3, 5, 6, 7},
	{2, 4, 5, 6, 7},
	{3, 4, 5, 6, 7}
};

void Player::initialize(
	ocilib::Connection& con,
	Logger* logger,
	PokerState* pokerState,
	std::vector<PlayerState>* playerStates,
	unsigned int seatNumber,
	unsigned int strategyId,
	unsigned int playerId,
	unsigned int buyInAmount
) {

	this->con = con;
	this->logger = logger;
	this->pokerState = pokerState;
	this->playerStates = playerStates;
	currentStrategyId = strategyId;

	thisPlayerState = &(playerStates->at(seatNumber - 1));
	thisPlayerState->state = PokerEnums::State::NO_MOVE;
	thisPlayerState->seatNumber = seatNumber;
	thisPlayerState->playerId = playerId;
	thisPlayerState->assumedStrategyId = 0;
	thisPlayerState->money = buyInAmount;
	thisPlayerState->tournamentRank = 0;
	resetGameState();
}

void Player::load(ocilib::Connection& con, Logger* logger, PokerState* pokerState, std::vector<PlayerState>* playerStates, ocilib::Resultset& playerStateRs) {

	this->con = con;
	this->logger = logger;
	this->pokerState = pokerState;
	this->playerStates = playerStates;

	// player attributes
	currentStrategyId = playerStateRs.Get<unsigned int>("current_strategy_id");
	holeCards.clear();
	int holeCard1 = playerStateRs.Get<int>("hole_card_1");
	if (holeCard1 != 0)
		holeCards.push_back(pokerState->deck.getCardById(holeCard1));
	int holeCard2 = playerStateRs.Get<int>("hole_card_2");
	if (holeCard2 != 0)
		holeCards.push_back(pokerState->deck.getCardById(holeCard2));
	bestHand.classification = (PokerEnums::HandClassification) playerStateRs.Get<unsigned int>("best_hand_classification");
	bestHand.comparator = playerStateRs.Get<std::string>("best_hand_comparator");
	bestHand.cards.clear();
	int bestHandCard1 = playerStateRs.Get<int>("best_hand_card_1");
	if (bestHandCard1 != 0) {
		bestHand.cards.push_back(pokerState->deck.getCardById(bestHandCard1));
		bestHand.cards.push_back(pokerState->deck.getCardById(playerStateRs.Get<int>("best_hand_card_2")));
		bestHand.cards.push_back(pokerState->deck.getCardById(playerStateRs.Get<int>("best_hand_card_3")));
		bestHand.cards.push_back(pokerState->deck.getCardById(playerStateRs.Get<int>("best_hand_card_4")));
		bestHand.cards.push_back(pokerState->deck.getCardById(playerStateRs.Get<int>("best_hand_card_5")));
		bestHandRank = playerStateRs.Get<unsigned int>("best_hand_rank");
	}
	else
		bestHandRank = 0;

	// player state
	unsigned int seatNumber = playerStateRs.Get<unsigned int>("seat_number");
	thisPlayerState = &(playerStates->at(seatNumber - 1));
	thisPlayerState->seatNumber = seatNumber;
	thisPlayerState->playerId = playerStateRs.Get<unsigned int>("player_id");
	thisPlayerState->assumedStrategyId = playerStateRs.Get<unsigned int>("assumed_strategy_id");
	thisPlayerState->handShowing = playerStateRs.Get<unsigned int>("hand_showing") == 1;
	thisPlayerState->presentedBetOpportunity = playerStateRs.Get<unsigned int>("presented_bet_opportunity") == 1;
	thisPlayerState->money = playerStateRs.Get<unsigned int>("money");
	thisPlayerState->state = (PokerEnums::State) playerStateRs.Get<unsigned int>("state");
	thisPlayerState->gameRank = playerStateRs.Get<unsigned int>("game_rank");
	thisPlayerState->tournamentRank = playerStateRs.Get<unsigned int>("tournament_rank");
	thisPlayerState->gamesPlayed = playerStateRs.Get<unsigned int>("games_played");
	thisPlayerState->mainPotsWon = playerStateRs.Get<unsigned int>("main_pots_won");
	thisPlayerState->mainPotsSplit = playerStateRs.Get<unsigned int>("main_pots_split");
	thisPlayerState->sidePotsWon = playerStateRs.Get<unsigned int>("side_pots_won");
	thisPlayerState->sidePotsSplit = playerStateRs.Get<unsigned int>("side_pots_split");
	thisPlayerState->averageGameProfit = playerStateRs.Get<float>("average_game_profit");
	thisPlayerState->flopsSeen = playerStateRs.Get<unsigned int>("flops_seen");
	thisPlayerState->turnsSeen = playerStateRs.Get<unsigned int>("turns_seen");
	thisPlayerState->riversSeen = playerStateRs.Get<unsigned int>("rivers_seen");
	thisPlayerState->preFlopFolds = playerStateRs.Get<unsigned int>("pre_flop_folds");
	thisPlayerState->flopFolds = playerStateRs.Get<unsigned int>("flop_folds");
	thisPlayerState->turnFolds = playerStateRs.Get<unsigned int>("turn_folds");
	thisPlayerState->riverFolds = playerStateRs.Get<unsigned int>("river_folds");
	thisPlayerState->totalFolds = playerStateRs.Get<unsigned int>("total_folds");
	thisPlayerState->preFlopChecks = playerStateRs.Get<unsigned int>("pre_flop_checks");
	thisPlayerState->flopChecks = playerStateRs.Get<unsigned int>("flop_checks");
	thisPlayerState->turnChecks = playerStateRs.Get<unsigned int>("turn_checks");
	thisPlayerState->riverChecks = playerStateRs.Get<unsigned int>("river_checks");
	thisPlayerState->totalChecks = playerStateRs.Get<unsigned int>("total_checks");
	thisPlayerState->preFlopCalls = playerStateRs.Get<unsigned int>("pre_flop_calls");
	thisPlayerState->flopCalls = playerStateRs.Get<unsigned int>("flop_calls");
	thisPlayerState->turnCalls = playerStateRs.Get<unsigned int>("turn_calls");
	thisPlayerState->riverCalls = playerStateRs.Get<unsigned int>("river_calls");
	thisPlayerState->totalCalls = playerStateRs.Get<unsigned int>("total_calls");
	thisPlayerState->preFlopBets = playerStateRs.Get<unsigned int>("pre_flop_bets");
	thisPlayerState->flopBets = playerStateRs.Get<unsigned int>("flop_bets");
	thisPlayerState->turnBets = playerStateRs.Get<unsigned int>("turn_bets");
	thisPlayerState->riverBets = playerStateRs.Get<unsigned int>("river_bets");
	thisPlayerState->totalBets = playerStateRs.Get<unsigned int>("total_bets");
	thisPlayerState->preFlopTotalBetAmount = playerStateRs.Get<unsigned int>("pre_flop_total_bet_amount");
	thisPlayerState->flopTotalBetAmount = playerStateRs.Get<unsigned int>("flop_total_bet_amount");
	thisPlayerState->turnTotalBetAmount = playerStateRs.Get<unsigned int>("turn_total_bet_amount");
	thisPlayerState->riverTotalBetAmount = playerStateRs.Get<unsigned int>("river_total_bet_amount");
	thisPlayerState->totalBetAmount = playerStateRs.Get<unsigned int>("total_bet_amount");
	thisPlayerState->preFlopAverageBetAmount = playerStateRs.Get<float>("pre_flop_average_bet_amount");
	thisPlayerState->flopAverageBetAmount = playerStateRs.Get<float>("flop_average_bet_amount");
	thisPlayerState->turnAverageBetAmount = playerStateRs.Get<float>("turn_average_bet_amount");
	thisPlayerState->riverAverageBetAmount = playerStateRs.Get<float>("river_average_bet_amount");
	thisPlayerState->averageBetAmount = playerStateRs.Get<float>("average_bet_amount");
	thisPlayerState->preFlopRaises = playerStateRs.Get<unsigned int>("pre_flop_raises");
	thisPlayerState->flopRaises = playerStateRs.Get<unsigned int>("flop_raises");
	thisPlayerState->turnRaises = playerStateRs.Get<unsigned int>("turn_raises");
	thisPlayerState->riverRaises = playerStateRs.Get<unsigned int>("river_raises");
	thisPlayerState->totalRaises = playerStateRs.Get<unsigned int>("total_raises");
	thisPlayerState->preFlopTotalRaiseAmount = playerStateRs.Get<unsigned int>("pre_flop_total_raise_amount");
	thisPlayerState->flopTotalRaiseAmount = playerStateRs.Get<unsigned int>("flop_total_raise_amount");
	thisPlayerState->turnTotalRaiseAmount = playerStateRs.Get<unsigned int>("turn_total_raise_amount");
	thisPlayerState->riverTotalRaiseAmount = playerStateRs.Get<unsigned int>("river_total_raise_amount");
	thisPlayerState->totalRaiseAmount = playerStateRs.Get<unsigned int>("total_raise_amount");
	thisPlayerState->preFlopAverageRaiseAmount = playerStateRs.Get<float>("pre_flop_average_raise_amount");
	thisPlayerState->flopAverageRaiseAmount = playerStateRs.Get<float>("flop_average_raise_amount");
	thisPlayerState->turnAverageRaiseAmount = playerStateRs.Get<float>("turn_average_raise_amount");
	thisPlayerState->riverAverageRaiseAmount = playerStateRs.Get<float>("river_average_raise_amount");
	thisPlayerState->averageRaiseAmount = playerStateRs.Get<float>("average_raise_amount");
	thisPlayerState->timesAllIn = playerStateRs.Get<unsigned int>("times_all_in");
	thisPlayerState->totalMoneyPlayed = playerStateRs.Get<unsigned int>("total_money_played");
	thisPlayerState->totalMoneyWon = playerStateRs.Get<unsigned int>("total_money_won");

}

bool Player::getIsActive() const {
	return !(thisPlayerState->state == PokerEnums::State::OUT_OF_TOURNAMENT);
}

PokerEnums::State Player::getState() const {
	return thisPlayerState->state;
}

std::string Player::getStateString() const {

	if (thisPlayerState->state == PokerEnums::State::NO_PLAYER)
		return "No Player";
	if (thisPlayerState->state == PokerEnums::State::NO_MOVE)
		return "No Move";
	if (thisPlayerState->state == PokerEnums::State::FOLDED)
		return "Folded";
	if (thisPlayerState->state == PokerEnums::State::CHECKED)
		return "Checked";
	if (thisPlayerState->state == PokerEnums::State::CALLED)
		return "Called";
	if (thisPlayerState->state == PokerEnums::State::MADE_BET)
		return "Bet";
	if (thisPlayerState->state == PokerEnums::State::RAISED)
		return "Raised";
	if (thisPlayerState->state == PokerEnums::State::OUT_OF_TOURNAMENT)
		return "Out of Tournament";
	if (thisPlayerState->state == PokerEnums::State::ALL_IN)
		return "All In";

	return "";
}

bool Player::getPresentedBetOpportunity() const {
	if (thisPlayerState->state != PokerEnums::State::OUT_OF_TOURNAMENT
		&& thisPlayerState->state != PokerEnums::State::FOLDED
		&& thisPlayerState->state != PokerEnums::State::ALL_IN)
		return thisPlayerState->presentedBetOpportunity;

	return true;
}

std::string Player::getBestHandComparator() const {
	return bestHand.comparator;
}

int Player::getMoney() const {
	return thisPlayerState->money;
}

void Player::getUiState(Json::Value& playerStateData) const {

	playerStateData["seat_number"] = thisPlayerState->seatNumber;
	if(thisPlayerState->playerId == 0)
		playerStateData["player_id"] = Json::Value::null;
	else
		playerStateData["player_id"] = thisPlayerState->playerId;
	if (holeCards.size() == 0) {
		playerStateData["hole_card_1"] = Json::Value::null;
		playerStateData["hole_card_2"] = Json::Value::null;
	}
	else {
		playerStateData["hole_card_1"] = holeCards[0].cardId;
		playerStateData["hole_card_2"] = holeCards[1].cardId;

	}
	if(bestHandRank == 0)
		playerStateData["best_hand_classification"] = Json::Value::null;
	else
		playerStateData["best_hand_classification"] = std::to_string(bestHandRank) + " - " + getHandClassificationString(bestHand.classification);

	if (bestHand.cards.size() == 0) {
		playerStateData["best_hand_card_1"] = Json::Value::null;
		playerStateData["best_hand_card_2"] = Json::Value::null;
		playerStateData["best_hand_card_3"] = Json::Value::null;
		playerStateData["best_hand_card_4"] = Json::Value::null;
		playerStateData["best_hand_card_5"] = Json::Value::null;
		playerStateData["best_hand_card_1_is_hole_card"] = Json::Value::null;
		playerStateData["best_hand_card_2_is_hole_card"] = Json::Value::null;
		playerStateData["best_hand_card_3_is_hole_card"] = Json::Value::null;
		playerStateData["best_hand_card_4_is_hole_card"] = Json::Value::null;
		playerStateData["best_hand_card_5_is_hole_card"] = Json::Value::null;
	}
	else {
		playerStateData["best_hand_card_1"] = bestHand.cards[0].cardId;
		playerStateData["best_hand_card_2"] = bestHand.cards[1].cardId;
		playerStateData["best_hand_card_3"] = bestHand.cards[2].cardId;
		playerStateData["best_hand_card_4"] = bestHand.cards[3].cardId;
		playerStateData["best_hand_card_5"] = bestHand.cards[4].cardId;
		playerStateData["best_hand_card_1_is_hole_card"] = bestHand.cards[0].cardId == holeCards[0].cardId || bestHand.cards[0].cardId == holeCards[1].cardId;
		playerStateData["best_hand_card_2_is_hole_card"] = bestHand.cards[1].cardId == holeCards[0].cardId || bestHand.cards[1].cardId == holeCards[1].cardId;
		playerStateData["best_hand_card_3_is_hole_card"] = bestHand.cards[2].cardId == holeCards[0].cardId || bestHand.cards[2].cardId == holeCards[1].cardId;
		playerStateData["best_hand_card_4_is_hole_card"] = bestHand.cards[3].cardId == holeCards[0].cardId || bestHand.cards[3].cardId == holeCards[1].cardId;
		playerStateData["best_hand_card_5_is_hole_card"] = bestHand.cards[4].cardId == holeCards[0].cardId || bestHand.cards[4].cardId == holeCards[1].cardId;

	}
	playerStateData["hand_showing"] = thisPlayerState->handShowing ? "Yes" : "No";
	playerStateData["money"] = thisPlayerState->money;
	playerStateData["state"] = getStateString();
	if (thisPlayerState->gameRank == 0)
		playerStateData["game_rank"] = Json::Value::null;
	else
		playerStateData["game_rank"] = thisPlayerState->gameRank;
	if (thisPlayerState->tournamentRank == 0)
		playerStateData["tournament_rank"] = Json::Value::null;
	else
		playerStateData["tournament_rank"] = thisPlayerState->tournamentRank;
	playerStateData["total_pot_contribution"] = pokerState->potController.getTotalPotContribution(thisPlayerState->seatNumber);
	playerStateData["can_fold"] = getCanFold();
	playerStateData["can_check"] = getCanCheck();
	playerStateData["can_call"] = getCanCall();
	playerStateData["can_bet"] = getCanBet();
	playerStateData["min_bet_amount"] = getMinBetAmount();
	playerStateData["max_bet_amount"] = getMaxBetAmount();
	playerStateData["can_raise"] = getCanRaise();
	playerStateData["min_raise_amount"] = getMinRaiseAmount();
	playerStateData["max_raise_amount"] = getMaxRaiseAmount();
}

void Player::issueWinnings(unsigned int winningsAmount, bool isMainPot, bool splittingPot) {

	thisPlayerState->money += winningsAmount;
	thisPlayerState->totalMoneyWon += winningsAmount;
	if (isMainPot) {
		if (splittingPot)
			thisPlayerState->mainPotsSplit++;
		else
			thisPlayerState->mainPotsWon++;
	}
	else {
		if (splittingPot)
			thisPlayerState->sidePotsSplit++;
		else
			thisPlayerState->sidePotsWon++;
	}

}

void Player::setBestHandRank(unsigned int rank) {
	bestHandRank = rank;
}

void Player::setHandShowing() {
	thisPlayerState->handShowing = true;
}

void Player::setPresentedBetOpportunity() {
	thisPlayerState->presentedBetOpportunity = true;
}

void Player::setHoleCards(Deck::Card holeCard1, Deck::Card holeCard2) {
	holeCards.resize(2);
	holeCards[0] = holeCard1;
	holeCards[1] = holeCard2;
	thisPlayerState->gamesPlayed++;
}

void Player::setPlayerShowdownMuck() {
	// debug - decision procedure to determine whether or not to show hand
	thisPlayerState->handShowing = true;
}

std::string Player::calculateBestHand() {

	// return if incomplete hand
	unsigned int communityCardCount = pokerState->communityCards.size();
	if (communityCardCount == 0 || thisPlayerState->state == PokerEnums::State::FOLDED || thisPlayerState->state == PokerEnums::State::OUT_OF_TOURNAMENT)
		return "";

	// collect possible complete hands from current hole and community cards
	std::vector<Hand> handCombinations;
	for (unsigned int i = 0; i < 21; i++) {

		Hand hand;
		for (unsigned int j = 0; j < 5; j++) {
			unsigned int cardIndex = possibleHandCombinations[i][j] - 1;
			if (cardIndex <= 4) {
				// card index refers to a community card, 0 - 4
				if (cardIndex < communityCardCount)
					hand.cards.push_back(pokerState->communityCards[cardIndex]);
			}
			else {
				// card index refers to a hole card, 5 - 6
				hand.cards.push_back(holeCards[cardIndex - 5]);
			}
		}
		if (hand.cards.size() == 5)
			handCombinations.push_back(hand);
	}

	// complete hands have been determined, calculate hand attributes
	for (unsigned int i = 0; i < handCombinations.size(); i++) {
		handCombinations[i] = calculateHandAttributes(handCombinations[i].cards);
	}

	// sort by hand comparator
	std::sort(handCombinations.begin(), handCombinations.end(), [](Hand i, Hand j) {
		return i.comparator < j.comparator;
	});

	// pull out best hand
	bestHand = handCombinations[handCombinations.size() - 1];

	return bestHand.comparator;
}

PokerEnums::State Player::performPlayerMove(PokerEnums::PlayerMove playerMove, unsigned int playerMoveAmount) {
	
	if (playerMove == PokerEnums::PlayerMove::AUTO) {
		performAutomaticPlayerMove();
	}
	else {
		performExplicitPlayerMove(playerMove, playerMoveAmount);
	}

	return thisPlayerState->state;
}

void Player::processGameResults(unsigned int tournamentRank) {

	if (thisPlayerState->state != PokerEnums::State::OUT_OF_TOURNAMENT) {

		// update average game profit
		thisPlayerState->averageGameProfit = ((float) (thisPlayerState->totalMoneyWon - thisPlayerState->totalMoneyPlayed)) / thisPlayerState->gamesPlayed;

		// mark as out of tournament if ran out of money
		if (thisPlayerState->money == 0) {
			thisPlayerState->tournamentRank = tournamentRank;
			thisPlayerState->state = PokerEnums::State::OUT_OF_TOURNAMENT;
			thisPlayerState->presentedBetOpportunity = false;
		}
	}

}

void Player::resetGameState() {
	holeCards.clear();
	bestHand.classification = PokerEnums::HandClassification::INCOMPLETE_HAND;
	bestHand.comparator = "";
	bestHand.cards.clear();
	bestHandRank = 0;
	thisPlayerState->handShowing = false;
	thisPlayerState->presentedBetOpportunity = false;
	thisPlayerState->gameRank = 0;

	if (thisPlayerState->state != PokerEnums::State::OUT_OF_TOURNAMENT)
		thisPlayerState->state = PokerEnums::State::NO_MOVE;
}

void Player::resetBettingRoundState() {
	if (thisPlayerState->state != PokerEnums::State::OUT_OF_TOURNAMENT && thisPlayerState->state != PokerEnums::State::FOLDED) {
		if (thisPlayerState->state != PokerEnums::State::ALL_IN) {
			thisPlayerState->state = PokerEnums::State::NO_MOVE;
			thisPlayerState->presentedBetOpportunity = false;
		}
	}

	if (pokerState->currentBettingRound == PokerEnums::BettingRound::FLOP) {
		thisPlayerState->flopsSeen++;
	}
	else if (pokerState->currentBettingRound == PokerEnums::BettingRound::TURN) {
		thisPlayerState->turnsSeen++;
	}
	else if (pokerState->currentBettingRound == PokerEnums::BettingRound::RIVER) {
		thisPlayerState->riversSeen++;
	}
}

void Player::insertStateLog() {

	std::string procCall = "BEGIN pkg_poker_ai.insert_player_state_log(";
	procCall.append("p_state_id                    => :stateId, ");
	procCall.append("p_seat_number                 => :seatNumber, ");
	procCall.append("p_player_id                   => :playerId, ");
	procCall.append("p_current_strategy_id         => :currentStrategyId, ");
	procCall.append("p_assumed_strategy_id         => :assumedStrategyId, ");
	procCall.append("p_hole_card_1                 => :holeCard1, ");
	procCall.append("p_hole_card_2                 => :holeCard2, ");
	procCall.append("p_best_hand_classification    => :bestHandClassification, ");
	procCall.append("p_best_hand_comparator        => :bestHandComparator, ");
	procCall.append("p_best_hand_card_1            => :bestHandCard1, ");
	procCall.append("p_best_hand_card_2            => :bestHandCard2, ");
	procCall.append("p_best_hand_card_3            => :bestHandCard3, ");
	procCall.append("p_best_hand_card_4            => :bestHandCard4, ");
	procCall.append("p_best_hand_card_5            => :bestHandCard5, ");
	procCall.append("p_best_hand_rank              => :bestHandRank, ");
	procCall.append("p_hand_showing                => :handShowing, ");
	procCall.append("p_presented_bet_opportunity   => :presentedBetOpportunity, ");
	procCall.append("p_money                       => :money, ");
	procCall.append("p_state                       => :state, ");
	procCall.append("p_game_rank                   => :gameRank, ");
	procCall.append("p_tournament_rank             => :tournamentRank, ");
	procCall.append("p_games_played                => :gamesPlayed, ");
	procCall.append("p_main_pots_won               => :mainPotsWon, ");
	procCall.append("p_main_pots_split             => :mainPotsSplit, ");
	procCall.append("p_side_pots_won               => :sidePotsWon, ");
	procCall.append("p_side_pots_split             => :sidePotsSplit, ");
	procCall.append("p_average_game_profit         => :averageGameProfit, ");
	procCall.append("p_flops_seen                  => :flopsSeen, ");
	procCall.append("p_turns_seen                  => :turnsSeen, ");
	procCall.append("p_rivers_seen                 => :riversSeen, ");
	procCall.append("p_pre_flop_folds              => :preFlopFolds, ");
	procCall.append("p_flop_folds                  => :flopFolds, ");
	procCall.append("p_turn_folds                  => :turnFolds, ");
	procCall.append("p_river_folds                 => :riverFolds, ");
	procCall.append("p_total_folds                 => :totalFolds, ");
	procCall.append("p_pre_flop_checks             => :preFlopChecks, ");
	procCall.append("p_flop_checks                 => :flopChecks, ");
	procCall.append("p_turn_checks                 => :turnChecks, ");
	procCall.append("p_river_checks                => :riverChecks, ");
	procCall.append("p_total_checks                => :totalChecks, ");
	procCall.append("p_pre_flop_calls              => :preFlopCalls, ");
	procCall.append("p_flop_calls                  => :flopCalls, ");
	procCall.append("p_turn_calls                  => :turnCalls, ");
	procCall.append("p_river_calls                 => :riverCalls, ");
	procCall.append("p_total_calls                 => :totalCalls, ");
	procCall.append("p_pre_flop_bets               => :preFlopBets, ");
	procCall.append("p_flop_bets                   => :flopBets, ");
	procCall.append("p_turn_bets                   => :turnBets, ");
	procCall.append("p_river_bets                  => :riverBets, ");
	procCall.append("p_total_bets                  => :totalBets, ");
	procCall.append("p_pre_flop_total_bet_amount   => :preFlopTotalBetAmount, ");
	procCall.append("p_flop_total_bet_amount       => :flopTotalBetAmount, ");
	procCall.append("p_turn_total_bet_amount       => :turnTotalBetAmount, ");
	procCall.append("p_river_total_bet_amount      => :riverTotalBetAmount, ");
	procCall.append("p_total_bet_amount            => :totalBetAmount, ");
	procCall.append("p_pre_flop_average_bet_amount => :preFlopAverageBetAmount, ");
	procCall.append("p_flop_average_bet_amount     => :flopAverageBetAmount, ");
	procCall.append("p_turn_average_bet_amount     => :turnAverageBetAmount, ");
	procCall.append("p_river_average_bet_amount    => :riverAverageBetAmount, ");
	procCall.append("p_average_bet_amount          => :averageBetAmount, ");
	procCall.append("p_pre_flop_raises             => :preFlopRaises, ");
	procCall.append("p_flop_raises                 => :flopRaises, ");
	procCall.append("p_turn_raises                 => :turnRaises, ");
	procCall.append("p_river_raises                => :riverRaises, ");
	procCall.append("p_total_raises                => :totalRaises, ");
	procCall.append("p_pre_flop_total_raise_amount => :preFlopTotalRaiseAmount, ");
	procCall.append("p_flop_total_raise_amount     => :flopTotalRaiseAmount, ");
	procCall.append("p_turn_total_raise_amount     => :turnTotalRaiseAmount, ");
	procCall.append("p_river_total_raise_amount    => :riverTotalRaiseAmount, ");
	procCall.append("p_total_raise_amount          => :totalRaiseAmount, ");
	procCall.append("p_pre_flop_average_raise_amt  => :preFlopAverageRaiseAmount, ");
	procCall.append("p_flop_average_raise_amount   => :flopAverageRaiseAmount, ");
	procCall.append("p_turn_average_raise_amount   => :turnAverageRaiseAmount, ");
	procCall.append("p_river_average_raise_amount  => :riverAverageRaiseAmount, ");
	procCall.append("p_average_raise_amount        => :averageRaiseAmount, ");
	procCall.append("p_times_all_in                => :timesAllIn, ");
	procCall.append("p_total_money_played          => :totalMoneyPlayed, ");
	procCall.append("p_total_money_won             => :totalMoneyWon");
	procCall.append("); END;");
	ocilib::Statement st(con);
	st.Prepare(procCall);

	st.Bind("stateId", pokerState->currentStateId, ocilib::BindInfo::In);
	st.Bind("seatNumber", thisPlayerState->seatNumber, ocilib::BindInfo::In);
	st.Bind("playerId", thisPlayerState->playerId, ocilib::BindInfo::In);
	if (thisPlayerState->playerId == 0)
		st.GetBind("playerId").SetDataNull(true, 1);
	st.Bind("currentStrategyId", currentStrategyId, ocilib::BindInfo::In);
	if (currentStrategyId == 0)
		st.GetBind("currentStrategyId").SetDataNull(true, 1);
	st.Bind("assumedStrategyId", thisPlayerState->assumedStrategyId, ocilib::BindInfo::In);
	if (thisPlayerState->assumedStrategyId == 0)
		st.GetBind("assumedStrategyId").SetDataNull(true, 1);
	int holeCard1 = holeCards.size() == 0 ? -1 : holeCards[0].cardId;
	st.Bind("holeCard1", holeCard1, ocilib::BindInfo::In);
	if(holeCard1 == -1)
		st.GetBind("holeCard1").SetDataNull(true, 1);
	int holeCard2 = holeCards.size() == 0 ? -1 : holeCards[1].cardId;
	st.Bind("holeCard2", holeCard2, ocilib::BindInfo::In);
	if (holeCard2 == -1)
		st.GetBind("holeCard2").SetDataNull(true, 1);
	unsigned int bestHandClassification = (unsigned int) bestHand.classification;
	st.Bind("bestHandClassification", bestHandClassification, ocilib::BindInfo::In);
	if (bestHandClassification == 0)
		st.GetBind("bestHandClassification").SetDataNull(true, 1);
	ocilib::ostring bestHandComparatorOstring(bestHand.comparator);
	st.Bind("bestHandComparator", bestHandComparatorOstring, static_cast<unsigned int>(bestHandComparatorOstring.size()), ocilib::BindInfo::In);
	if (bestHand.classification == PokerEnums::HandClassification::INCOMPLETE_HAND) {
		int cardId = -1;
		st.Bind("bestHandCard1", cardId, ocilib::BindInfo::In);
		st.Bind("bestHandCard2", cardId, ocilib::BindInfo::In);
		st.Bind("bestHandCard3", cardId, ocilib::BindInfo::In);
		st.Bind("bestHandCard4", cardId, ocilib::BindInfo::In);
		st.Bind("bestHandCard5", cardId, ocilib::BindInfo::In);
		st.GetBind("bestHandCard1").SetDataNull(true, 1);
		st.GetBind("bestHandCard2").SetDataNull(true, 1);
		st.GetBind("bestHandCard3").SetDataNull(true, 1);
		st.GetBind("bestHandCard4").SetDataNull(true, 1);
		st.GetBind("bestHandCard5").SetDataNull(true, 1);
	}
	else {
		st.Bind("bestHandCard1", bestHand.cards[0].cardId, ocilib::BindInfo::In);
		st.Bind("bestHandCard2", bestHand.cards[1].cardId, ocilib::BindInfo::In);
		st.Bind("bestHandCard3", bestHand.cards[2].cardId, ocilib::BindInfo::In);
		st.Bind("bestHandCard4", bestHand.cards[3].cardId, ocilib::BindInfo::In);
		st.Bind("bestHandCard5", bestHand.cards[4].cardId, ocilib::BindInfo::In);
	}
	st.Bind("bestHandRank", bestHandRank, ocilib::BindInfo::In);
	if(bestHandRank == 0)
		st.GetBind("bestHandRank").SetDataNull(true, 1);
	unsigned int handShowing(thisPlayerState->handShowing ? 1 : 0);
	st.Bind("handShowing", handShowing, ocilib::BindInfo::In);
	unsigned int presentedBetOpportunity(thisPlayerState->presentedBetOpportunity ? 1 : 0);
	st.Bind("presentedBetOpportunity", presentedBetOpportunity, ocilib::BindInfo::In);
	st.Bind("money", thisPlayerState->money, ocilib::BindInfo::In);
	unsigned int state = (unsigned int) thisPlayerState->state;
	st.Bind("state", state, ocilib::BindInfo::In);
	st.Bind("gameRank", thisPlayerState->gameRank, ocilib::BindInfo::In);
	if (thisPlayerState->gameRank == 0)
		st.GetBind("gameRank").SetDataNull(true, 1);
	st.Bind("tournamentRank", thisPlayerState->tournamentRank, ocilib::BindInfo::In);
	if (thisPlayerState->tournamentRank == 0)
		st.GetBind("tournamentRank").SetDataNull(true, 1);
	st.Bind("gamesPlayed", thisPlayerState->gamesPlayed, ocilib::BindInfo::In);
	st.Bind("mainPotsWon", thisPlayerState->mainPotsWon, ocilib::BindInfo::In);
	st.Bind("mainPotsSplit", thisPlayerState->mainPotsSplit, ocilib::BindInfo::In);
	st.Bind("sidePotsWon", thisPlayerState->sidePotsWon, ocilib::BindInfo::In);
	st.Bind("sidePotsSplit", thisPlayerState->sidePotsSplit, ocilib::BindInfo::In);
	st.Bind("averageGameProfit", thisPlayerState->averageGameProfit, ocilib::BindInfo::In);
	if(thisPlayerState->gamesPlayed == 0)
		st.GetBind("averageGameProfit").SetDataNull(true, 1);
	st.Bind("flopsSeen", thisPlayerState->flopsSeen, ocilib::BindInfo::In);
	st.Bind("turnsSeen", thisPlayerState->turnsSeen, ocilib::BindInfo::In);
	st.Bind("riversSeen", thisPlayerState->riversSeen, ocilib::BindInfo::In);
	st.Bind("preFlopFolds", thisPlayerState->preFlopFolds, ocilib::BindInfo::In);
	st.Bind("flopFolds", thisPlayerState->flopFolds, ocilib::BindInfo::In);
	st.Bind("turnFolds", thisPlayerState->turnFolds, ocilib::BindInfo::In);
	st.Bind("riverFolds", thisPlayerState->riverFolds, ocilib::BindInfo::In);
	st.Bind("totalFolds", thisPlayerState->totalFolds, ocilib::BindInfo::In);
	st.Bind("preFlopChecks", thisPlayerState->preFlopChecks, ocilib::BindInfo::In);
	st.Bind("flopChecks", thisPlayerState->flopChecks, ocilib::BindInfo::In);
	st.Bind("turnChecks", thisPlayerState->turnChecks, ocilib::BindInfo::In);
	st.Bind("riverChecks", thisPlayerState->riverChecks, ocilib::BindInfo::In);
	st.Bind("totalChecks", thisPlayerState->totalChecks, ocilib::BindInfo::In);
	st.Bind("preFlopCalls", thisPlayerState->preFlopCalls, ocilib::BindInfo::In);
	st.Bind("flopCalls", thisPlayerState->flopCalls, ocilib::BindInfo::In);
	st.Bind("turnCalls", thisPlayerState->turnCalls, ocilib::BindInfo::In);
	st.Bind("riverCalls", thisPlayerState->riverCalls, ocilib::BindInfo::In);
	st.Bind("totalCalls", thisPlayerState->totalCalls, ocilib::BindInfo::In);
	st.Bind("preFlopBets", thisPlayerState->preFlopBets, ocilib::BindInfo::In);
	st.Bind("flopBets", thisPlayerState->flopBets, ocilib::BindInfo::In);
	st.Bind("turnBets", thisPlayerState->turnBets, ocilib::BindInfo::In);
	st.Bind("riverBets", thisPlayerState->riverBets, ocilib::BindInfo::In);
	st.Bind("totalBets", thisPlayerState->totalBets, ocilib::BindInfo::In);
	st.Bind("preFlopTotalBetAmount", thisPlayerState->preFlopTotalBetAmount, ocilib::BindInfo::In);
	st.Bind("flopTotalBetAmount", thisPlayerState->flopTotalBetAmount, ocilib::BindInfo::In);
	st.Bind("turnTotalBetAmount", thisPlayerState->turnTotalBetAmount, ocilib::BindInfo::In);
	st.Bind("riverTotalBetAmount", thisPlayerState->riverTotalBetAmount, ocilib::BindInfo::In);
	st.Bind("totalBetAmount", thisPlayerState->totalBetAmount, ocilib::BindInfo::In);
	st.Bind("preFlopAverageBetAmount", thisPlayerState->preFlopAverageBetAmount, ocilib::BindInfo::In);
	if(thisPlayerState->preFlopBets == 0)
		st.GetBind("preFlopAverageBetAmount").SetDataNull(true, 1);
	st.Bind("flopAverageBetAmount", thisPlayerState->flopAverageBetAmount, ocilib::BindInfo::In);
	if (thisPlayerState->flopBets == 0)
		st.GetBind("flopAverageBetAmount").SetDataNull(true, 1);
	st.Bind("turnAverageBetAmount", thisPlayerState->turnAverageBetAmount, ocilib::BindInfo::In);
	if (thisPlayerState->turnBets == 0)
		st.GetBind("turnAverageBetAmount").SetDataNull(true, 1);
	st.Bind("riverAverageBetAmount", thisPlayerState->riverAverageBetAmount, ocilib::BindInfo::In);
	if (thisPlayerState->riverBets == 0)
		st.GetBind("riverAverageBetAmount").SetDataNull(true, 1);
	st.Bind("averageBetAmount", thisPlayerState->averageBetAmount, ocilib::BindInfo::In);
	if (thisPlayerState->totalBets == 0)
		st.GetBind("averageBetAmount").SetDataNull(true, 1);
	st.Bind("preFlopRaises", thisPlayerState->preFlopRaises, ocilib::BindInfo::In);
	st.Bind("flopRaises", thisPlayerState->flopRaises, ocilib::BindInfo::In);
	st.Bind("turnRaises", thisPlayerState->turnRaises, ocilib::BindInfo::In);
	st.Bind("riverRaises", thisPlayerState->riverRaises, ocilib::BindInfo::In);
	st.Bind("totalRaises", thisPlayerState->totalRaises, ocilib::BindInfo::In);
	st.Bind("preFlopTotalRaiseAmount", thisPlayerState->preFlopTotalRaiseAmount, ocilib::BindInfo::In);
	st.Bind("flopTotalRaiseAmount", thisPlayerState->flopTotalRaiseAmount, ocilib::BindInfo::In);
	st.Bind("turnTotalRaiseAmount", thisPlayerState->turnTotalRaiseAmount, ocilib::BindInfo::In);
	st.Bind("riverTotalRaiseAmount", thisPlayerState->riverTotalRaiseAmount, ocilib::BindInfo::In);
	st.Bind("totalRaiseAmount", thisPlayerState->totalRaiseAmount, ocilib::BindInfo::In);
	st.Bind("preFlopAverageRaiseAmount", thisPlayerState->preFlopAverageRaiseAmount, ocilib::BindInfo::In);
	if (thisPlayerState->preFlopRaises == 0)
		st.GetBind("preFlopAverageRaiseAmount").SetDataNull(true, 1);
	st.Bind("flopAverageRaiseAmount", thisPlayerState->flopAverageRaiseAmount, ocilib::BindInfo::In);
	if (thisPlayerState->flopRaises == 0)
		st.GetBind("flopAverageRaiseAmount").SetDataNull(true, 1);
	st.Bind("turnAverageRaiseAmount", thisPlayerState->turnAverageRaiseAmount, ocilib::BindInfo::In);
	if (thisPlayerState->turnRaises == 0)
		st.GetBind("turnAverageRaiseAmount").SetDataNull(true, 1);
	st.Bind("riverAverageRaiseAmount", thisPlayerState->riverAverageRaiseAmount, ocilib::BindInfo::In);
	if (thisPlayerState->riverRaises == 0)
		st.GetBind("riverAverageRaiseAmount").SetDataNull(true, 1);
	st.Bind("averageRaiseAmount", thisPlayerState->averageRaiseAmount, ocilib::BindInfo::In);
	if (thisPlayerState->totalRaises == 0)
		st.GetBind("averageRaiseAmount").SetDataNull(true, 1);
	st.Bind("timesAllIn", thisPlayerState->timesAllIn, ocilib::BindInfo::In);
	st.Bind("totalMoneyPlayed", thisPlayerState->totalMoneyPlayed, ocilib::BindInfo::In);
	st.Bind("totalMoneyWon", thisPlayerState->totalMoneyWon, ocilib::BindInfo::In);
	st.ExecutePrepared();

}

inline bool Player::getCanFold() const {

	return thisPlayerState->seatNumber == pokerState->turnSeatNumber
		&& pokerState->bettingRoundInProgress
		&& thisPlayerState->state != PokerEnums::State::OUT_OF_TOURNAMENT
		&& thisPlayerState->state != PokerEnums::State::FOLDED
		&& thisPlayerState->state != PokerEnums::State::ALL_IN;

}

inline bool Player::getCanCheck() const {

	return thisPlayerState->seatNumber == pokerState->turnSeatNumber
		&& pokerState->bettingRoundInProgress
		&& thisPlayerState->state != PokerEnums::State::OUT_OF_TOURNAMENT
		&& thisPlayerState->state != PokerEnums::State::FOLDED
		&& thisPlayerState->state != PokerEnums::State::ALL_IN
		&& pokerState->potController.getPotDeficit(thisPlayerState->seatNumber) == 0;

}

inline bool Player::getCanCall() const {

	return thisPlayerState->seatNumber == pokerState->turnSeatNumber
		&& pokerState->bettingRoundInProgress
		&& thisPlayerState->state != PokerEnums::State::OUT_OF_TOURNAMENT
		&& thisPlayerState->state != PokerEnums::State::FOLDED
		&& thisPlayerState->state != PokerEnums::State::ALL_IN
		&& pokerState->potController.getPotDeficit(thisPlayerState->seatNumber) > 0;

}

bool Player::getCanBet() const {

	bool playerStateOk = thisPlayerState->seatNumber == pokerState->turnSeatNumber
		&& pokerState->bettingRoundInProgress
		&& thisPlayerState->state != PokerEnums::State::OUT_OF_TOURNAMENT
		&& thisPlayerState->state != PokerEnums::State::FOLDED
		&& thisPlayerState->state != PokerEnums::State::ALL_IN
		&& !pokerState->potController.getBetExists(pokerState->currentBettingRound)
		&& pokerState->potController.getPotDeficit(thisPlayerState->seatNumber) == 0;

	if (playerStateOk) {
		// at least one peer player must be able to respond to the bet
		for (unsigned int i = 0; i < pokerState->playerCount; i++) {
			PlayerState* ps = &playerStates->at(i);
			if (ps->seatNumber != thisPlayerState->seatNumber
				&& ps->state != PokerEnums::State::OUT_OF_TOURNAMENT
				&& ps->state != PokerEnums::State::FOLDED
				&& ps->state != PokerEnums::State::ALL_IN
				) {
				return true;
			}
		}
	}

	return false;
}

bool Player::getCanRaise() const {

	bool playerStateOk = thisPlayerState->seatNumber == pokerState->turnSeatNumber
		&& pokerState->bettingRoundInProgress
		&& thisPlayerState->state != PokerEnums::State::OUT_OF_TOURNAMENT
		&& thisPlayerState->state != PokerEnums::State::FOLDED
		&& thisPlayerState->state != PokerEnums::State::ALL_IN
		&& pokerState->potController.getBetExists(pokerState->currentBettingRound)
		&& (thisPlayerState->money - pokerState->potController.getPotDeficit(thisPlayerState->seatNumber) > 0);

	if (playerStateOk) {
		// at least one peer player must be able to respond to the raise
		for (unsigned int i = 0; i < pokerState->playerCount; i++) {
			PlayerState* ps = &playerStates->at(i);
			if (ps->seatNumber != thisPlayerState->seatNumber
				&& ps->state != PokerEnums::State::OUT_OF_TOURNAMENT
				&& ps->state != PokerEnums::State::FOLDED
				&& ps->state != PokerEnums::State::ALL_IN
				) {
				return true;
			}
		}
	}

	return false;
}

inline unsigned int Player::getMinBetAmount() const {
	if (thisPlayerState->money < (int) pokerState->minRaiseAmount)
		return thisPlayerState->money;
	else
		return pokerState->minRaiseAmount;
}

unsigned int Player::getMaxBetAmount() const {

	// determine max amount of money among peers
	int maxPeerMoney = -1;
	for (unsigned int i = 0; i < pokerState->playerCount; i++) {
		PlayerState* ps = &playerStates->at(i);
		if (ps->seatNumber != thisPlayerState->seatNumber
			&& ps->state != PokerEnums::State::OUT_OF_TOURNAMENT
			&& ps->state != PokerEnums::State::FOLDED
			&& ps->money > maxPeerMoney
			) {
			maxPeerMoney = ps->money;
		}
	}

	if (maxPeerMoney < thisPlayerState->money)
		return maxPeerMoney;

	return thisPlayerState->money;
}

inline unsigned int Player::getMinRaiseAmount() const {

	int remainingMoney = thisPlayerState->money - pokerState->potController.getPotDeficit(thisPlayerState->seatNumber);

	if (remainingMoney >= (int) pokerState->minRaiseAmount)
		return pokerState->minRaiseAmount;

	return thisPlayerState->money;
}

unsigned int Player::getMaxRaiseAmount() const {

	int remainingMoney = thisPlayerState->money - pokerState->potController.getPotDeficit(thisPlayerState->seatNumber);

	// determine max amount of money among peers
	int maxPeerMoney = -1;
	for (unsigned int i = 0; i < pokerState->playerCount; i++) {
		PlayerState* ps = &playerStates->at(i);
		if (ps->seatNumber != thisPlayerState->seatNumber
			&& ps->state != PokerEnums::State::OUT_OF_TOURNAMENT
			&& ps->state != PokerEnums::State::FOLDED
			) {
			int peerMoney = ps->money - pokerState->potController.getPotDeficit(ps->seatNumber);
			if (peerMoney > maxPeerMoney)
				maxPeerMoney = peerMoney;
		}
	}

	if (remainingMoney < maxPeerMoney)
		return remainingMoney;

	return maxPeerMoney;
}

std::string Player::getHandClassificationString(PokerEnums::HandClassification classification) const {

	if (classification == PokerEnums::HandClassification::INCOMPLETE_HAND)
		return "Incomplete Hand";
	else if (classification == PokerEnums::HandClassification::HIGH_CARD)
		return "High Card";
	else if (classification == PokerEnums::HandClassification::ONE_PAIR)
		return "One Pair";
	else if (classification == PokerEnums::HandClassification::TWO_PAIR)
		return "Two Pair";
	else if (classification == PokerEnums::HandClassification::THREE_OF_A_KIND)
		return "Three of a Kind";
	else if (classification == PokerEnums::HandClassification::STRAIGHT)
		return "Straight";
	else if (classification == PokerEnums::HandClassification::FLUSH)
		return "Flush";
	else if (classification == PokerEnums::HandClassification::FULL_HOUSE)
		return "Full House";
	else if (classification == PokerEnums::HandClassification::FOUR_OF_A_KIND)
		return "Four of a Kind";
	else if (classification == PokerEnums::HandClassification::STRAIGHT_FLUSH)
		return "Straight Flush";
	else if (classification == PokerEnums::HandClassification::ROYAL_FLUSH)
		return "Royal Flush";

	return "";
}

Player::Hand Player::calculateHandAttributes(std::vector<Deck::Card> cards) {

	// input cards will be analyzed and stored in seperate collection with additional attributes used for classification and sorting
	struct CardAttributes {
		Deck::Card card;
		unsigned int valueOccurences;
	};
	std::vector<CardAttributes> analyzedCards;

	// gather attributes for hand analysis
	std::unordered_map<Deck::Suit, bool> distinctSuits;
	std::unordered_map<unsigned int, bool> distinctValues;
	unsigned int highCardValue = 0;
	for (unsigned int i = 0; i < 5; i++) {

		// initialize analyzed card structure
		CardAttributes cardAttributes;
		cardAttributes.card = cards[i];
		cardAttributes.valueOccurences = 1;

		// check prior analyzed cards for additional occurences of this card's value, update if found
		for (unsigned int j = 0; j < analyzedCards.size(); j++) {
			CardAttributes* analyzedCard = &analyzedCards[j];
			if (analyzedCard->card.value == cardAttributes.card.value) {
				analyzedCard->valueOccurences++;
				cardAttributes.valueOccurences++;
			}
		}

		// capture distict suits
		distinctSuits[cardAttributes.card.suit] = true;

		// capture distinct values
		distinctValues[cardAttributes.card.value] = true;

		// capture highest card value
		if (cardAttributes.card.value > highCardValue)
			highCardValue = cardAttributes.card.value;

		// store analyzed card
		analyzedCards.push_back(cardAttributes);
	}
	unsigned int distinctSuitCount = distinctSuits.size();
	unsigned int distinctValueCount = distinctValues.size();

	// determine max number of value occurences
	unsigned int maxValueOccurences = 0;
	for (unsigned int i = 0; i < analyzedCards.size(); i++) {
		unsigned int valueOccurences = analyzedCards[i].valueOccurences;
		if (valueOccurences > maxValueOccurences)
			maxValueOccurences = valueOccurences;
	}

	// determine if hand is a straight
	std::sort(cards.begin(), cards.end(), [](Deck::Card left, Deck::Card right) {return left.value < right.value;});
	unsigned int straightCardCount;
	for (straightCardCount = 0; straightCardCount < 4; straightCardCount++) {
		if (cards[straightCardCount].value + 1 != cards[straightCardCount + 1].value) {
			break;
		}
	}
	bool isStraight = straightCardCount == 4;
	bool straightAceLow = false;
	if (!isStraight) {
		// may be a straight with ace low
		if (cards[0].value == 2 && cards[1].value == 3 && cards[2].value == 4 && cards[3].value == 5 && cards[4].value == 14) {
			isStraight = true;
			straightAceLow = true;
		}
	}

	// classify hand
	Hand hand;
	if (isStraight && distinctSuitCount == 1 && highCardValue == 14)
		hand.classification = PokerEnums::HandClassification::ROYAL_FLUSH;
	else if (isStraight && distinctSuitCount == 1)
		hand.classification = PokerEnums::HandClassification::STRAIGHT_FLUSH;
	else if (maxValueOccurences == 4)
		hand.classification = PokerEnums::HandClassification::FOUR_OF_A_KIND;
	else if (maxValueOccurences == 3 && distinctValueCount == 2)
		hand.classification = PokerEnums::HandClassification::FULL_HOUSE;
	else if (distinctSuitCount == 1)
		hand.classification = PokerEnums::HandClassification::FLUSH;
	else if (isStraight)
		hand.classification = PokerEnums::HandClassification::STRAIGHT;
	else if (maxValueOccurences == 3)
		hand.classification = PokerEnums::HandClassification::THREE_OF_A_KIND;
	else if (maxValueOccurences == 2 && distinctValueCount == 3)
		hand.classification = PokerEnums::HandClassification::TWO_PAIR;
	else if (maxValueOccurences == 2)
		hand.classification = PokerEnums::HandClassification::ONE_PAIR;
	else
		hand.classification = PokerEnums::HandClassification::HIGH_CARD;

	// sort the cards according to the hand's classification
	if (hand.classification == PokerEnums::HandClassification::HIGH_CARD
		|| hand.classification == PokerEnums::HandClassification::ONE_PAIR
		|| hand.classification == PokerEnums::HandClassification::TWO_PAIR
		|| hand.classification == PokerEnums::HandClassification::THREE_OF_A_KIND
		|| hand.classification == PokerEnums::HandClassification::FULL_HOUSE
		|| hand.classification == PokerEnums::HandClassification::FOUR_OF_A_KIND
	) {

		// sort the analyzed cards by value occurence descending, then card value descending, then suit descending
		std::sort(analyzedCards.begin(), analyzedCards.end(), [](CardAttributes left, CardAttributes right) {
			if (left.valueOccurences == right.valueOccurences) {
				if (left.card.value == right.card.value)
					return left.card.suit > right.card.suit;
				else
					return left.card.value > right.card.value;
			}
			else
				return left.valueOccurences > right.valueOccurences;
		});

	}
	else if (hand.classification == PokerEnums::HandClassification::STRAIGHT
		|| hand.classification == PokerEnums::HandClassification::STRAIGHT_FLUSH
		|| hand.classification == PokerEnums::HandClassification::ROYAL_FLUSH
	) {

		if (straightAceLow) {
			// find the ace and change the value to indicate low value
			for (unsigned int i = 0; i < analyzedCards.size(); i++) {
				Deck::Card* card = &analyzedCards[i].card;
				if (card->value == 14) {
					card->value = 1;
					break;
				}
			}
		}

		// sort the cards by value ascending
		std::sort(analyzedCards.begin(), analyzedCards.end(), [](CardAttributes left, CardAttributes right) {
			return left.card.value < right.card.value;
		});

	}
	else if (hand.classification == PokerEnums::HandClassification::FLUSH) {

		// sort the cards by value descending
		std::sort(analyzedCards.begin(), analyzedCards.end(), [](CardAttributes left, CardAttributes right) {
			return left.card.value > right.card.value;
		});

	}

	// set cards in hand in sorted order and build comparator string including card values for tie breaking
	hand.comparator = "";
	for (unsigned int i = 0; i < 5; i++) {
		hand.cards.push_back(analyzedCards[i].card);
		hand.comparator += "_" + Util::zeroPadNumber(analyzedCards[i].card.value);
	}
	hand.comparator = Util::zeroPadNumber(hand.classification) + hand.comparator;

	return hand;
}

void Player::performAutomaticPlayerMove() {

	if (currentStrategyId == 0) {
		// perform random move
		logger->log(pokerState->currentStateId, "player at seat " + std::to_string(thisPlayerState->seatNumber) + " is performing random move");

		std::vector<PokerEnums::PlayerMove> possiblePlayerMoves;
		if (getCanFold())
			possiblePlayerMoves.push_back(PokerEnums::PlayerMove::FOLD);
		if(getCanCheck())
			possiblePlayerMoves.push_back(PokerEnums::PlayerMove::CHECK);
		if(getCanCall())
			possiblePlayerMoves.push_back(PokerEnums::PlayerMove::CALL);
		if(getCanBet())
			possiblePlayerMoves.push_back(PokerEnums::PlayerMove::BET);
		if(getCanRaise())
			possiblePlayerMoves.push_back(PokerEnums::PlayerMove::RAISE);

		unsigned int randomPlayerMoveIndex = rand() % possiblePlayerMoves.size();
		PokerEnums::PlayerMove playerMove = possiblePlayerMoves[randomPlayerMoveIndex];
		unsigned int playerMoveAmount = 0;

		if (playerMove == PokerEnums::PlayerMove::BET) {
			unsigned int minBetAmount = getMinBetAmount();
			unsigned int maxBetAmount = getMaxBetAmount();
			playerMoveAmount = (rand() % (maxBetAmount - minBetAmount + 1)) + minBetAmount;
		}
		else if (playerMove == PokerEnums::PlayerMove::RAISE) {
			unsigned int minRaiseAmount = getMinRaiseAmount();
			unsigned int maxRaiseAmount = getMaxRaiseAmount();
			playerMoveAmount = (rand() % (maxRaiseAmount - minRaiseAmount + 1)) + minRaiseAmount;
		}
		
		performExplicitPlayerMove(playerMove, playerMoveAmount);
	}
	else {
		// use current strategy
		logger->log(pokerState->currentStateId, "player at seat " + std::to_string(thisPlayerState->seatNumber) + " is deriving move from strategy procedure");
	}

}

void Player::performExplicitPlayerMove(PokerEnums::PlayerMove playerMove, unsigned int playerMoveAmount) {

	if (playerMove == PokerEnums::PlayerMove::FOLD) {
		logger->log(pokerState->currentStateId, "player at seat " + std::to_string(thisPlayerState->seatNumber) + " folds");
		thisPlayerState->state = PokerEnums::State::FOLDED;
		thisPlayerState->updateStatFold(pokerState->currentBettingRound);
		pokerState->potController.issueApplicablePotRefunds(pokerState->currentStateId);
		pokerState->potController.issueDefaultPotWins(pokerState->currentStateId);
	}
	else if (playerMove == PokerEnums::PlayerMove::CHECK) {
		logger->log(pokerState->currentStateId, "player at seat " + std::to_string(thisPlayerState->seatNumber) + " checks");
		if(thisPlayerState->state != PokerEnums::State::ALL_IN)
			thisPlayerState->state = PokerEnums::State::CHECKED;
		thisPlayerState->updateStatCheck(pokerState->currentBettingRound);
	}
	else if (playerMove == PokerEnums::PlayerMove::CALL) {
		logger->log(pokerState->currentStateId, "player at seat " + std::to_string(thisPlayerState->seatNumber) + " calls");

		int potDeficit = pokerState->potController.getPotDeficit(thisPlayerState->seatNumber);
		unsigned int contributionAmount = thisPlayerState->money < potDeficit ? thisPlayerState->money : potDeficit;
		pokerState->potController.contributeToPot(thisPlayerState->seatNumber, contributionAmount, pokerState->currentBettingRound, pokerState->currentStateId);

		if (thisPlayerState->state != PokerEnums::State::ALL_IN)
			thisPlayerState->state = PokerEnums::State::CALLED;
		thisPlayerState->updateStatCall(pokerState->currentBettingRound);
	}
	else if (playerMove == PokerEnums::PlayerMove::BET) {
		logger->log(pokerState->currentStateId, "player at seat " + std::to_string(thisPlayerState->seatNumber) + " bets " + std::to_string(playerMoveAmount));
		pokerState->potController.contributeToPot(thisPlayerState->seatNumber, playerMoveAmount, pokerState->currentBettingRound, pokerState->currentStateId);
		if (thisPlayerState->state != PokerEnums::State::ALL_IN)
			thisPlayerState->state = PokerEnums::State::MADE_BET;
		thisPlayerState->updateStatBet(pokerState->currentBettingRound, playerMoveAmount);

		pokerState->lastToRaiseSeatNumber = thisPlayerState->seatNumber;
		pokerState->minRaiseAmount = playerMoveAmount < pokerState->minRaiseAmount ? (playerMoveAmount + pokerState->minRaiseAmount) : playerMoveAmount;
	}
	else if (playerMove == PokerEnums::PlayerMove::RAISE) {
		logger->log(pokerState->currentStateId, "player at seat " + std::to_string(thisPlayerState->seatNumber) + " raises " + std::to_string(playerMoveAmount));

		unsigned int potDeficit = pokerState->potController.getPotDeficit(thisPlayerState->seatNumber);
		pokerState->potController.contributeToPot(thisPlayerState->seatNumber, potDeficit + playerMoveAmount, pokerState->currentBettingRound, pokerState->currentStateId);

		if (thisPlayerState->state != PokerEnums::State::ALL_IN)
			thisPlayerState->state = PokerEnums::State::RAISED;
		thisPlayerState->updateStatRaise(pokerState->currentBettingRound, playerMoveAmount);

		pokerState->lastToRaiseSeatNumber = thisPlayerState->seatNumber;
		pokerState->minRaiseAmount = playerMoveAmount < pokerState->minRaiseAmount ? (playerMoveAmount + pokerState->minRaiseAmount) : playerMoveAmount;
	}

}
