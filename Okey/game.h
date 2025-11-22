typedef struct {
    int number;
    int color;
} DeckPiece;

DeckPiece deck[106] = {0};
int piecesLeft = 106;

void InitDeck() {
    int index = 0;

    for (int color = 0; color < 4; color++) {
        for (int number = 1; number <= 13; number++) {
            for (int i = 0; i < 2; i++) {
                deck[index++] = (DeckPiece){number, color};
            }
        }
    }

    deck[104] = (DeckPiece){14, 0};
    deck[105] = (DeckPiece){14, 0};
}

void ShuffleDeck() {
    for (int i = 105; i > 0; i--) {
        int j = GetRandomValue(0, i);
        DeckPiece temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

DeckPiece DrawFromDeck() {
    if(piecesLeft > 0) {
        DeckPiece new = deck[piecesLeft - 1];
        piecesLeft--;
        return new;
    } else {
        return (DeckPiece){0, 0};
    }
}

void DrawNewHand() {
    for (int i = 0; i < 21; i++) {
        DeckPiece dp = DrawFromDeck();

        hand[i].number = dp.number;
        hand[i].color = dp.color;

        hand[i].currentPos = (Vector2){ 400, 300 };
        hand[i].targetPos = GetPiecePosition(i);
        hand[i].animProgress = 0.0f;
    }

    for (int i = 21; i < 32; i++) {
        hand[i].number = 0;
        hand[i].color = 0;
        hand[i].currentPos = GetPiecePosition(i);
        hand[i].targetPos = hand[i].currentPos;
        hand[i].animProgress = 1.0f;
    }
}

bool isSequence(Piece* pieces, int start, int length) {
    if (length < 3) return false;
    int color = pieces[start].color;

    for (int i = start; i < start + length; i++) {
        if (pieces[i].color != color ||
            pieces[i].number == 0 ||
            (i > start && pieces[i].number != pieces[i-1].number + 1)) {
            return false;
        }
    }
    return true;
}


bool isGroup(Piece* pieces, int start, int length) {
    if (length < 3 || length > 4) return false;
    int number = pieces[start].number;
    if (number == 0) return false;

    bool used_colors[4] = {false};
    for (int i = start; i < start + length; i++) {
        if (pieces[i].number != number || used_colors[pieces[i].color]) {
            return false;
        }
        used_colors[pieces[i].color] = true;
    }
    return true;
}


int calculateCombinationValue(Piece* hand, int start, int length) {
    int sum = 0;
    for (int i = start; i < start + length; i++) {
        sum += hand[i].number;
    }
    return sum;
}

int calculateHandValue() {
    int totalValue = 0;
    int i = 0;

    while (i < 32) {

        if (hand[i].number == 0) {
            i++;
            continue;
        }

        bool foundCombination = false;


        for (int length = 13; length >= 3; length--) {
            if (i + length <= 32 && isSequence(hand, i, length)) {
                totalValue += calculateCombinationValue(hand, i, length);
                i += length;
                foundCombination = true;
                break;
            }
        }


        if (!foundCombination) {
            for (int length = 4; length >= 3; length--) {
                if (i + length <= 32 && isGroup(hand, i, length)) {
                    totalValue += calculateCombinationValue(hand, i, length);
                    i += length;
                    foundCombination = true;
                    break;
                }
            }
        }


        if (!foundCombination) {
            i++;
        }
    }

    return totalValue;
}


#define MAX_JOKER_COUNT 8
#define MAX_PURCHASED_JOKERS 6
#define MARKET_SLOTS 4

typedef struct {
    Texture texture;
    bool active;
    int price;
    char* name;
    char* desc;
    bool inMarket;
} Joker;

// Expanded joker pool
Joker jokers[MAX_JOKER_COUNT] = {
    {(Texture){0}, false, 3, "Girişimci",    "Her tur 2$ kazandırır", false},
    {(Texture){0}, false, 2, "Pisagor",      "Renk fark etmeksizin (3,4,5), (6,8,10) ve (5,12,13) \nsıralı taşları oynamana izin verir", false},
    {(Texture){0}, false, 2, "Dönme Dolap",  "Tıklandığında elindeki \n6 ve 9 numarı taşları döndürür", false},
    {(Texture){0}, false, 4, "Büyük Burhan", "Destedeki tüm taşların numarsını yarılar \n'Kurtuluş için küçük bir bedel'", false},
    {(Texture){0}, false, 1, "Bonus", "Her tur sonunda +15 bonus puan kazanırsın", false},
    {(Texture){0}, false, 6, "Çifte Kazanç", "Kazandığın para ikiye katlanır", false},
    {(Texture){0}, false, 4, "Kumarbaz", "%50 ihtimalle 2 kat puan kazandırır", false},
    {(Texture){0}, false, 2, "Ressam", "Her tur 1 kartın rengini değiştirebilrisin", false},

    // {(Texture){0}, false, 3, "Çılgın Joker", "Her el çekilişinde bir taşı joker olarak kullanabilirsin", false},
    // {(Texture){0}, false, 5, "Altın Eller",  "Bir seferlik tüm kombinasyonların değerini ikiye katlar", false},
    // {(Texture){0}, false, 4, "Zaman Ustası", "Bir el daha oynama hakkı verir", false},
    // {(Texture){0}, false, 5, "İkizler",      "Seçilen bir taşı dublör olarak kullanır", false},
};

int activeJokerCount = 0;
int currentMarketSize = 0;

void RefreshMarket() {
    // Reset market status
    for(int i = 0; i < MAX_JOKER_COUNT; i++) {
        jokers[i].inMarket = false;
    }

    currentMarketSize = 0;

    // Count available jokers
    int availableJokers = 0;
    for(int i = 0; i < MAX_JOKER_COUNT; i++) {
        if(!jokers[i].active) {
            availableJokers++;
        }
    }

    // Determine how many slots we can fill
    int slotsToFill = MARKET_SLOTS;
    if(availableJokers < MARKET_SLOTS) {
        slotsToFill = availableJokers;
    }

    // Fill available market slots with random jokers
    while(currentMarketSize < slotsToFill) {
        int randomIndex = GetRandomValue(0, MAX_JOKER_COUNT - 1);

        // Skip if joker is already active or in market
        if(!jokers[randomIndex].active && !jokers[randomIndex].inMarket) {
            jokers[randomIndex].inMarket = true;
            currentMarketSize++;
        }
    }
}


void DrawTooltip(const char* text, Vector2 position) {

    Vector2 textSize = MeasureTextEx(font32, text, 18, 0);

    float padding = 10.0f;
    Rectangle tooltipRect = {
        position.x - textSize.x / 2,
        position.y,
        textSize.x + (padding * 2),
        textSize.y + (padding * 2)
    };

    Color bgColor = (Color){60, 50, 40, 230};
    Color shadowColor = (Color){0, 0, 0, 100};

    DrawRectangleShadowed((Rectangle){tooltipRect.x, tooltipRect.y, tooltipRect.width, tooltipRect.height}, 0.2f, bgColor, shadowColor, (Vector2){0, 3});
    DrawTextEx(font18, text, (Vector2){position.x - textSize.x / 2 + padding, position.y + padding}, 18, 0, WHITE);
}


void InitGame() {
    InitDeck();
    ShuffleDeck();
    DrawNewHand();

    const char* textureFiles[] = {
        "assets/investor.png", "assets/pisagor.png", "assets/wheel.png", "assets/chart.png",
        "assets/plus15.png", "assets/double.png", "assets/gamble.png", "assets/palette.png",
        "assets/twins.png", "assets/color.png", "assets/stone.png", "assets/dice.png"
    };

    for(int i = 0; i < MAX_JOKER_COUNT; i++) {
        jokers[i].texture = LoadTexture(textureFiles[i]);
    }

    RefreshMarket();
}

void DrawJokerMarket() {
    // Draw market jokers
    int current = 0;
    for(int i = 0; i < MAX_JOKER_COUNT; i++) {
        if(jokers[i].inMarket) {
            Rectangle jokerRect = (Rectangle){ 850, 20 + 85 * current, 330, 70 };
            DrawRectangleShadowed(jokerRect, 0.3f,
                                (Color){140, 90, 50, 255},
                                (Color){130, 80, 40, 255},
                                (Vector2){ 0, 6 });

            DrawCircle(885, 56 + 85 * current, 23, PIECE_COLOR_SHADOW);
            DrawCircle(885, 53 + 85 * current, 23, PIECE_COLOR);
            DrawTextEx(font32, jokers[i].name, (Vector2){920, 38 + 85 * current}, 32, 0, WHITE);
            DrawTexturePro(jokers[i].texture,
                          (Rectangle){0, 0, 256, 256},
                          (Rectangle){869, 36 + 85 * current, 32, 32},
                          (Vector2){0, 0}, 0, WHITE);

            // Purchase button
            if(Button((Rectangle){ 1115, 30 + 85 * current, 55, 45 },
                     TextFormat("%d$", jokers[i].price)) &&
               money >= jokers[i].price &&
               activeJokerCount < MAX_PURCHASED_JOKERS) {
                jokers[i].active = true;
                jokers[i].inMarket = false;
                money -= jokers[i].price;
                activeJokerCount++;
                currentMarketSize--;
            }

            // Tooltip
            if(CheckCollisionPointCircle(GetMousePosition(), (Vector2){885, 53 + 85 * current}, 30)) {
                Vector2 tooltipPos = {
                    885, 35 + 85 * current - 23
                };
                DrawTooltip(jokers[i].desc, tooltipPos);
            }

            current++;
        }
    }

    current = 0;

    // Draw active jokers at the top
    for(int i = 0; i < MAX_JOKER_COUNT; i++) {
        if(jokers[i].active) {
            Rectangle activeJokerRect = (Rectangle){
                300 + 85 * current,
                20,
                76,
                76
            };
            DrawCircle(300 + 85 * current + 38, 23 + 38, 38, PIECE_COLOR_SHADOW);
            DrawCircle(300 + 85 * current + 38, 20 + 38, 38, PIECE_COLOR);
            DrawTexturePro(jokers[i].texture,
                          (Rectangle){0, 0, 256, 256},
                          (Rectangle){314 + 85 * current, 32, 48, 48},
                          (Vector2){0, 0}, 0, WHITE);

            // Draw tooltip on hover
            if(CheckCollisionPointCircle(GetMousePosition(),
                                      (Vector2){300 + 85 * current + 38, 20 + 38}, 38)) {
                Vector2 tooltipPos = {
                    activeJokerRect.x + 38,
                    activeJokerRect.y + activeJokerRect.height + 10
                };
                DrawTooltip(jokers[i].desc, tooltipPos);
            }
            current++;
        }
    }
}

void UpdateGame() {
    UpdateHand();
}

void DrawGame() {
    DrawUI();
    DrawHand();

    if(Button((Rectangle){ 880, 420, 270, 80 }, "Oyna")) {
        if(hands >= 0) {
            int currentScore = calculateHandValue();
            money += currentScore / 5;
            score += currentScore;
            hands -= 1;
            DrawNewHand();
            RefreshMarket();  // Refresh market each round

            if(hands == -1) {
                for(int i = 0; i < 32; i++) {
                    hand[i].number = 0;
                    hand[i].color = 0;
                    hand[i].currentPos = GetPiecePosition(i);
                    hand[i].targetPos = hand[i].currentPos;
                    hand[i].animProgress = 1.0f;
                }
            }
        }
    }

    DrawJokerMarket();

}