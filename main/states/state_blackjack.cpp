
#include "states.h"

#include <vector>
#include <iostream>

std::vector<std::string> suit_names = {" of Hearts", " of Diamonds", " of Clubs", " of Spades"};
std::vector<std::string> value_names = {"2", "3", "4", "5", "6", "7", "8"," 9", "10", "Jack", "Queen", "King", "Ace"};

std::vector<std::string> card_list = { // the '1' is supposed to be "10", but it's shortened to keep each card to 2 chars
    "2♥", "3♥", "4♥", "5♥", "6♥", "7♥", "8♥", "9♥", "1♥", "J♥", "Q♥", "K♥", "A♥",  // hearts
    "2♦", "3♦", "4♦", "5♦", "6♦", "7♦", "8♦", "9♦", "1♦", "J♦", "Q♦", "K♦", "A♦",  // diamonds
    "2♣", "3♣", "4♣", "5♣", "6♣", "7♣", "8♣", "9♣", "1♣", "J♣", "Q♣", "K♣", "A♣",  // clubs
    "2♠", "3♠", "4♠", "5♠", "6♠", "7♠", "8♠", "9♠", "1♠", "J♠", "Q♠", "K♠", "A♠"   // spades
};
// std::vector<std::string> card_list = {
//     "🂲", "🂳", "🂴", "🂵", "🂶", "🂷", "🂸", "🂹", "🂺", "🂻", "🂽", "🂾", "🂱",  // hearts
//     "🃂", "🃃", "🃄", "🃅", "🃆", "🃇", "🃈", "🃉", "🃊", "🃋", "🃍", "🃎", "🃁",  // diamonds
//     "🃒", "🃓", "🃔", "🃕", "🃖", "🃗", "🃘", "🃙", "🃚", "🃛", "🃝", "🃞", "🃑",  // clubs
//     "🂢", "🂣", "🂤", "🂥", "🂦", "🂧", "🂨", "🂩", "🂪", "🂫", "🂭", "🂮", "🂡"   // spades
// };
std::vector<std::string> card_image_filenames = {
    "2_of_hearts", "3_of_hearts", "4_of_hearts", "5_of_hearts", "6_of_hearts", "7_of_hearts", "8_of_hearts", "9_of_hearts", "10_of_hearts", "jack_of_hearts2", "queen_of_hearts2", "king_of_hearts2", "ace_of_hearts",
    "2_of_diamonds", "3_of_diamonds", "4_of_diamonds", "5_of_diamonds", "6_of_diamonds", "7_of_diamonds", "8_of_diamonds", "9_of_diamonds", "10_of_diamonds", "jack_of_diamonds2", "queen_of_diamonds2", "king_of_diamonds2", "ace_of_diamonds",
    "2_of_clubs", "3_of_clubs", "4_of_clubs", "5_of_clubs", "6_of_clubs", "7_of_clubs", "8_of_clubs", "9_of_clubs", "10_of_clubs", "jack_of_clubs2", "queen_of_clubs2", "king_of_clubs2", "ace_of_clubs",
    "2_of_spades", "3_of_spades", "4_of_spades", "5_of_spades", "6_of_spades", "7_of_spades", "8_of_spades", "9_of_spades", "10_of_spades", "jack_of_spades2", "queen_of_spades2", "king_of_spades2", "ace_of_spades",
};
std::vector<int> cards_deck;
std::vector<int> player_deck;
std::vector<int> cpu_deck;
int game_state = 0;
bool game_state_init = false;
bool player_stuck = false;
bool cpu_stuck = false;
uint16_t card_image_w = 55;
uint16_t card_image_h = 80;
uint16_t card_image_x = 0;
uint8_t card_image_padding = 5;

void reset_decks()
{
    cards_deck.clear();
    player_deck.clear();
    cpu_deck.clear();

    for (int i = 0; i < card_list.size(); i++) cards_deck.push_back(i);
}

int deal_card(std::vector<int> &deck)
{
    // pick a random card
    srand(time(NULL));
    int selected_card_id = rand() % cards_deck.size();
    
    // deal it to a player
    deck.push_back(cards_deck[selected_card_id]);

    // remove it from the main deck
    cards_deck.erase(cards_deck.begin() + selected_card_id);

    // return the card id
    return selected_card_id;
}

int tally_cards(std::vector<int> deck)
{
    int tally = 0;
    for (int i = 0; i < deck.size(); i++)
    {
        int index = deck[i] % 13;
        if (index < 8) tally += index + 2; // if the card is 2 to 9
        else tally += 10;
    }
    return tally;
}

void print_status(const char *msg, uint16_t colour, bool reset_cursor=true)
{
    if (reset_cursor) {
        watch2::oled.fillRect(0, 0, SCREEN_WIDTH, watch2::oled.fontHeight() * 6, BLACK);
        watch2::oled.setCursor(0, 0);
    }
    watch2::oled.setTextColor(colour, BLACK);
    watch2::oled.println(msg);
}

void switch_game_state(int state)
{
    game_state = state;
    game_state_init = false;
}

void draw_card(int card_id)
{
    // get filename
    std::string image_filename = "/images/cards/";
    image_filename += card_image_filenames[card_id];
    image_filename += ".jpg";

    // get image data
    watch2::imageData data = watch2::getImageData(image_filename.c_str());

    // if image loaded successfully
    if (data.data != NULL)
    {
        watch2::drawImage(data, card_image_x, SCREEN_HEIGHT - card_image_h);
        card_image_x += card_image_w + card_image_padding;
    }
    else 
    {
        ESP_LOGW(WATCH2_TAG, "failed to load image %s (%s)", image_filename.c_str(), data.error);
    }
    watch2::freeImageData(data.data);
}

void state_func_blackjack()
{
    if (!watch2::state_init)
    {
        reset_decks();
        int card = deal_card(player_deck);
        deal_card(cpu_deck);
        card_image_x = 0;
        switch_game_state(0);
    }

    if (game_state == 0) // title screen
    {
        if (game_state_init == false)
        {
            print_status("Blackjack\nPress up to exit\nPress down to start", CYAN);
            game_state_init = true;
        }

        if (dpad_up_active()) watch2::switchState(-1);
        if (dpad_down_active()) {
            switch_game_state(1);
            draw_card(player_deck[0]);
        }
    }

    if (game_state == 1) // player's turn
    {
        if (game_state_init == false)
        {
            print_status("Player's Turn", CYAN);
            print_status("Press left to twist\n or press right to stick", WHITE, false);
            player_stuck = false;
            cpu_stuck = false;
            game_state_init = true;
        }

        if (dpad_left_active())
        {
            // twist
            int card_id = deal_card(player_deck);
            
            std::string msg = "You got the ";
            msg += value_names[card_id % 13] + suit_names[card_id / 13];
            msg += "\nYou have ";
            msg += std::to_string(tally_cards(player_deck));
            msg += " points";

            print_status(msg.c_str(), WHITE, false);
            draw_card(card_id);
            delay(2000);
            switch_game_state(2);
        }

        if (dpad_right_active())
        {
            // stick
            print_status("You stuck", WHITE, false);
            player_stuck = true;
            delay(2000);
            switch_game_state(2);
        }

    }

    if (game_state == 2) // cpu turn
    {
        if (game_state_init == false)
        {
            print_status("CPU's Turn", RED);
            game_state_init = true;
            delay(2000 + (rand() % 2000));

            // stick or twist
            int cpu_choice;

            // if cpu is bust, stick
            if (tally_cards(cpu_deck) > 21) cpu_choice = 0;

            // otherwise randomly decided whether to stick or twist
            else cpu_choice = rand() % 2;
            
            if (cpu_choice == 0) // cpu stuck
            {
                cpu_stuck = true; 
                print_status("CPU Stuck", WHITE, false);
            }
            else // cpu twisted
            {
                deal_card(cpu_deck);
                print_status("CPU Twisted", WHITE, false);
            }

            // print cpu card count
            std::string msg = "CPU now has ";
            msg += std::to_string(cpu_deck.size());
            msg += " cards";
            print_status(msg.c_str(), WHITE, false);

            delay(2000);

            // if both players have stuck
            if (player_stuck && cpu_stuck)
            {
                switch_game_state(3);
            }
            else switch_game_state(1);
        }
    }

    if (game_state == 3) // results
    {
        if (game_state_init == false)
        {
            print_status("Both players have stuck...", GREEN);
            delay(2000);
            print_status("The winner is...", GREEN, false);
            delay(2000);

            int player_tally = tally_cards(player_deck);
            int cpu_tally = tally_cards(cpu_deck);

            if (player_tally > 21 && cpu_tally <= 21) print_status("CPU", RED, false);          // player is bust
            else if (player_tally <= 21 && cpu_tally > 21) print_status("You!", CYAN, false);   // cpu is bust
            else if (player_tally > 21 && cpu_tally > 21)  print_status("No one, it's a draw", WHITE, false); // both players are bust
            else {
                int player_difference = 21 - player_tally;
                int cpu_difference = 21 - cpu_tally;

                if (player_difference > cpu_difference) print_status("CPU", RED, false);        // player is closer to 21
                else if (player_difference < cpu_difference) print_status("You!", CYAN, false); //cpu is closer to 21
                else if (player_difference == cpu_difference)  print_status("No one, it's a draw", WHITE, false); // both players have the same score
            }

            std::string msg = "You had ";
            msg += std::to_string(player_tally) + " points\nThe CPU had " + std::to_string(cpu_tally) + " points";
            print_status(msg.c_str(), WHITE, false);

            print_status("Press down to continue\nor press up to exit", WHITE, false);

            game_state_init = true;
        }

        if (dpad_up_active()) watch2::switchState(-1);
        if (dpad_down_active()) {
            switch_game_state(0);
            watch2::switchState(watch2::state);
        }

    }
}