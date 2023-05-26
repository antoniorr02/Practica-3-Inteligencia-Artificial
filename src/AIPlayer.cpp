# include "AIPlayer.h"
# include "Parchis.h"

const double masinf = 9999999999.0, menosinf = -9999999999.0;
const double gana = masinf - 1, pierde = menosinf + 1;
const int num_pieces = 3;
const int PROFUNDIDAD_MINIMAX = 4;  // Umbral maximo de profundidad para el metodo MiniMax
const int PROFUNDIDAD_ALFABETA = 6; // Umbral maximo de profundidad para la poda Alfa_Beta

bool AIPlayer::move(){
    cout << "Realizo un movimiento automatico" << endl;

    color c_piece;
    int id_piece;
    int dice;
    think(c_piece, id_piece, dice);

    cout << "Movimiento elegido: " << str(c_piece) << " " << id_piece << " " << dice << endl;

    actual->movePiece(c_piece, id_piece, dice);
    return true;
}

void AIPlayer::thinkAleatorio(color &c_piece, int &id_piece, int &dice) const {
    // IMPLEMENTACIÓN INICIAL DEL AGENTE
    // Esta implementación realiza un movimiento aleatorio.
    // Se proporciona como ejemplo, pero se debe cambiar por una que realice un movimiento inteligente
    //como lo que se muestran al final de la función.

    // OBJETIVO: Asignar a las variables c_piece, id_piece, dice (pasadas por referencia) los valores,
    //respectivamente, de:
    // - color de ficha a mover
    // - identificador de la ficha que se va a mover
    // - valor del dado con el que se va a mover la ficha.

    // El id de mi jugador actual.
    int player = actual->getCurrentPlayerId();

    // Vector que almacenará los dados que se pueden usar para el movimiento
    vector<int> current_dices;
    // Vector que almacenará los ids de las fichas que se pueden mover para el dado elegido.
    vector<tuple<color, int>> current_pieces;

    // Se obtiene el vector de dados que se pueden usar para el movimiento
    current_dices = actual->getAvailableNormalDices(player);
    // Elijo un dado de forma aleatoria.
    dice = current_dices[rand() % current_dices.size()];

    // Se obtiene el vector de fichas que se pueden mover para el dado elegido
    current_pieces = actual->getAvailablePieces(player, dice);

    // Si tengo fichas para el dado elegido muevo una al azar.
    if (current_pieces.size() > 0) {
        int random_id = rand() % current_pieces.size();
        id_piece = get<1>(current_pieces[random_id]); // get<i>(tuple<...>) me devuelve el i-ésimo
        c_piece = get<0>(current_pieces[random_id]);  // elemento de la tupla
    } else {
        // Si no tengo fichas para el dado elegido, pasa turno (la macro SKIP_TURN me permite no mover).
        id_piece = SKIP_TURN;
        c_piece = actual->getCurrentColor(); // Le tengo que indicar mi color actual al pasar turno.
    }
}

void AIPlayer::thinkAleatorioMasInteligente(color &c_piece, int &id_piece, int &dice) const {
    // El número de mi jugador actual.
    int player = actual->getCurrentPlayerId();
    // Vector que almacenará los dados que se pueden usar para el movimiento.
    vector<int> current_dices_con_especiales;
    // Vector que almacenará los ids de las fichas que se pueden mover para el dado elegido.
    vector<tuple<color, int>> current_pieces;
    // Obtengo el vector de dados que puedo usar para el movimiento.
    // En este caso elijo todos, tanto normales como especiales.
    // Importante: puedo acceder a solo dados normales o especiales por separado,
    // o a todos a la vez:
    // - actual->getAvailableNormalDices(player) -> solo dados normales
    // - actual->getAvailableSpecialDices(player) -> solo dados especiales
    // - actual->getAllAvailableDices(player) -> todos los dados
    // Importante 2: los "available" me dan los dados que puedo usar en el turno actual.
    // Por ejemplo, si me tengo que contar 10 o 20 solo me saldrán los dados 10 y 20.
    // Puedo saber qué más dados tengo, aunque no los pueda usar en este turno, con:
    // - actual->getNormalDices(player) -> todos los dados normales
    // - actual->getSpecialDices(player) -> todos los dados especiales
    // - actual->getAllDices(player) -> todos los dados
    current_dices_con_especiales = actual->getAllAvailableDices(player);

    // En vez de elegir un dado al azar, miro primero cuáles tienen fichas que se puedan mover.
    vector<int> current_dices_que_pueden_mover_ficha;
    for (int i = 0; i < current_dices_con_especiales.size(); i++) {
        // Se obtiene el vector de fichas que se pueden mover para el dado elegido.
        current_pieces = actual->getAvailablePieces(player, current_dices_con_especiales[i]);
        // Si se pueden mover fichas para el dado actual, lo añado al vector de dados que pueden mover fichas.
        if (current_pieces.size() > 0) {
            current_dices_que_pueden_mover_ficha.push_back(current_dices_con_especiales[i]);
        }
    }
    // Si no tengo ninún dado que pueda mover fichas, paso turno con un dado al azar (la macro SKIP_TURN me permite no mover).
    if (current_dices_que_pueden_mover_ficha.size() == 0) {
        dice = current_dices_con_especiales[rand() % current_dices_con_especiales.size()];
        id_piece = SKIP_TURN;
        c_piece = actual->getCurrentColor(); // Le tengo que indicar mi color actual al pasar turno.
    }
    // En caso contrario, elijo un dado de forma aleatoria de entre los que pueden mover ficha.
    else {
        dice = current_dices_que_pueden_mover_ficha[rand() % current_dices_que_pueden_mover_ficha.size()];
        // Se obtiene el vector de fichas que se pueden mover para el dado elegido.
        current_pieces = actual->getAvailablePieces(player, dice);
        // Muevo una ficha al azar de entre las que se pueden mover.
        int random_id = rand() % current_pieces.size();
        id_piece = get<1>(current_pieces[random_id]);
        c_piece = get<0>(current_pieces[random_id]);
    }
}
void AIPlayer::thinkFichaMasAdelantada(color &c_piece, int &id_piece, int &dice) const {
    
    // Elijo el dado haciendo lo mismo que el jugador anterior.
    thinkAleatorioMasInteligente(c_piece, id_piece, dice);
    
    // Tras llamar a esta función, ya tengo en dice el número de dado que quiero usar.
    // Ahora, en vez de mover una ficha al azar, voy a mover (o a aplicar
    // el dado especial a) la que esté más adelantada
    // (equivalentemente, la más cercana a la meta).
    int player = actual->getCurrentPlayerId();
    vector<tuple<color, int>> current_pieces = actual->getAvailablePieces(player, dice);
    int id_ficha_mas_adelantada = -1;
    color col_ficha_mas_adelantada = none;
    int min_distancia_meta = 9999;
    for (int i = 0; i < current_pieces.size(); i++) {
        // distanceToGoal(color, id) devuelve la distancia a la meta de la ficha [id] del color que le indique.
        color col = get<0>(current_pieces[i]);
        int id = get<1>(current_pieces[i]);
        int distancia_meta = actual->distanceToGoal(col, id);
        if (distancia_meta < min_distancia_meta) {
            min_distancia_meta = distancia_meta;
            id_ficha_mas_adelantada = id;
            col_ficha_mas_adelantada = col;
        }
    }

    // Si no he encontrado ninguna ficha, paso turno.
    if (id_ficha_mas_adelantada == -1) {
        id_piece = SKIP_TURN;
        c_piece = actual->getCurrentColor(); // Le tengo que indicar mi color actual al pasar turno.
    }
    // En caso contrario, moveré la ficha más adelantada.
    else {
        id_piece = id_ficha_mas_adelantada;
        c_piece = col_ficha_mas_adelantada;
    }
}
void AIPlayer::thinkMejorOpcion(color &c_piece, int &id_piece, int &dice) const {
    // Vamos a mirar todos los posibles movimientos del jugador actual accediendo a los hijos del st actual.
    // Para ello, vamos a iterar sobre los hijos con la función de Parchis getChildren().
    // Esta función devuelve un objeto de la clase ParchisBros, que es una estructura iterable
    // sobre la que se pueden recorrer todos los hijos del st sobre el que se llama.
    ParchisBros hijos = actual->getChildren();
    bool me_quedo_con_esta_accion = false;
    // La clase ParchisBros viene con un iterador muy útil y sencillo de usar.
    // Al hacer begin() accedemos al primer hijo de la rama,
    // y cada vez que hagamos ++it saltaremos al siguiente hijo.
    // Comparando con el iterador end() podemos consultar cuándo hemos terminado de visitar los hijos.
    for(ParchisBros::Iterator it = hijos.begin(); it != hijos.end() and !me_quedo_con_esta_accion; ++it) {
        Parchis siguiente_hijo = *it; // Accedemos al tablero hijo con el operador de indirección.
        if(siguiente_hijo.isEatingMove() || siguiente_hijo.isGoalMove() || (siguiente_hijo.gameOver() and siguiente_hijo.getWinner() == this->jugador)) {
            me_quedo_con_esta_accion = true;
            c_piece = it.getMovedColor(); // Guardo color de la ficha movida.
            id_piece = it.getMovedPieceId(); // Guardo id de la ficha movida.
            dice = it.getMovedDiceValue(); // Guardo número de dado movido.
        }
    }
    // Si he encontrado una acción que me interesa, la guardo en las variables pasadas por referencia.
    // (Ya lo he hecho antes, cuando les he asignado los valores con el iterador).
    // Si no, muevo la ficha más adelantada como antes.
    if(!me_quedo_con_esta_accion) {
        thinkFichaMasAdelantada(c_piece, id_piece, dice);
    }
}

double AIPlayer::Heuristica3(const Parchis &estado, int jugador) {
// Heurística de prueba proporcionada para validar el funcionamiento del algoritmo de búsqueda.


    int ganador = estado.getWinner();
    int oponente = (jugador + 1) % 2;

    // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
    if (ganador == jugador)
    {
        return gana;
    }
    else if (ganador == oponente)
    {
        return pierde;
    }
    else
    {
        // Colores que juega mi jugador y colores del oponente
        vector<color> my_colors = estado.getPlayerColors(jugador);
        vector<color> op_colors = estado.getPlayerColors(oponente);

        // Recorro todas las fichas de mi jugador
        int puntuacion_jugador = 0;
        // Recorro colores de mi jugador.
        for (int i = 0; i < my_colors.size(); i++)
        {
            color c = my_colors[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++)
            {
                // Valoro positivamente que la ficha esté en casilla segura o meta.
                if (estado.isSafePiece(c, j))
                {
                    puntuacion_jugador++;
                }
                else if (estado.getBoard().getPiece(c, j).get_box().type == goal)
                {
                    puntuacion_jugador += 5;
                }
            }
            // Si nos comemos una ficha.
            if(estado.isEatingMove() && estado.getCurrentPlayerId() == jugador){
                if(estado.eatenPiece().first != my_colors[(i+1)%2]){
                    puntuacion_jugador += 3;
                }
            }

            if (estado.piecesDestroyedLastMove().size() > 0 && estado.getCurrentPlayerId() == jugador) {
                for (int p = 0; p < estado.piecesDestroyedLastMove().size(); p++) {
                    if (estado.piecesDestroyedLastMove()[p].first != my_colors[(i+1)%2]) {
                        puntuacion_jugador++;
                    }
                }
            }
        }

        // Recorro todas las fichas del oponente
        int puntuacion_oponente = 0;
        // Recorro colores del oponente.
        for (int i = 0; i < op_colors.size(); i++)
        {
            color c = op_colors[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++)
            {
                if (estado.isSafePiece(c, j))
                {
                    // Valoro negativamente que la ficha esté en casilla segura o meta.
                    puntuacion_oponente++;
                }
                else if (estado.getBoard().getPiece(c, j).get_box().type == goal)
                {
                    puntuacion_oponente += 5;
                }
            }
            // Si nos comemos una ficha.
            if(estado.isEatingMove() && estado.getCurrentPlayerId() == oponente){
                if(estado.eatenPiece().first != op_colors[(i+1)%2]){
                    puntuacion_oponente += 3;
                }
            }

            if (estado.piecesDestroyedLastMove().size() > 0 && estado.getCurrentPlayerId() == oponente) {
                for (int p = 0; p < estado.piecesDestroyedLastMove().size(); p++) {
                    if (estado.piecesDestroyedLastMove()[p].first != op_colors[(i+1)%2]) {
                        puntuacion_oponente++;
                    }
                }
            }
        }

        // Devuelvo la puntuación de mi jugador menos la puntuación del oponente.
        return puntuacion_jugador - puntuacion_oponente;
    }
}


double AIPlayer::Heuristica2(const Parchis &st, int jugador) {
    int ganador = st.getWinner();
    int oponente = (jugador + 1) % 2;

    // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
    if (ganador == jugador) {
        return gana;
    } else if (ganador == oponente) {
        return pierde;
    } else {
        // Colores que juega mi jugador y colores del oponente
        vector<color> coloresJugador = st.getPlayerColors(jugador);
        vector<color> coloresOponente = st.getPlayerColors(oponente);

        double valoracionJugador = 0;
        // Recorro colores de mi jugador.
        for (int i = 0; i < coloresJugador.size(); i++) {
            color c = coloresJugador[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++) {
                // Si ponemos una ficha en una casilla segura.
                if (st.isSafePiece(c, j)) {
                    valoracionJugador += 10;
                }
                // Si nos comen:
                if (st.getBoard().getPiece(c, j).get_box().type == home) { //////////////////////// DUDA.
                    valoracionJugador -= 50;
                }                
            }            

            // Consigue dado especial.
            if (st.itemAcquired() && st.getCurrentPlayerId() == jugador) {
                if (st.getItemAcquired() == star || st.getItemAcquired() == bullet) {  // dar puntuación a cada objeto.
                    valoracionJugador += 25;
                } else if (st.getItemAcquired() == mega_mushroom || st.getItemAcquired() == red_shell || st.getItemAcquired() == blue_shell) { 
                    valoracionJugador += 15;
                } else {
                    valoracionJugador += 10;
                }
            }

            // Fantasma.
            if (st.isBooMove() && st.getCurrentPlayerId() == jugador) {
                valoracionJugador += 15;
            }

            // Cohete.
            if (st.isBulletMove() && st.getCurrentPlayerId() == jugador) {
                valoracionJugador += 35;
                /*if (st.anyWall(std::get<2>(st.getLastMoves().back()), std::get<3>(st.getLastMoves().back())).size() > 0) {
                    valoracionJugador += 10; // Atraviesa barrera.
                }
                if (st.anyMegaWall(std::get<2>(st.getLastMoves().back()), std::get<3>(st.getLastMoves().back())).size() > 0) {
                    valoracionJugador += 20; // Atraviesa mega-barrera.
                }*/
            }
 
            // Si nos comemos una ficha.
            if(st.isEatingMove() && st.getCurrentPlayerId() == jugador){
                if(st.eatenPiece().first == coloresJugador[(i+1)%2]){
                    // Si nos comemos una ficha de nuestro equipo pero estamos más cerca de ganar con el color que comemos --> Positivo
                    if (st.piecesAtGoal(st.eatenPiece().first) > st.piecesAtGoal(st.getCurrentColor()))
                        valoracionJugador -= 10;
                    else
                        valoracionJugador += 10;
                } else {
                    valoracionJugador += 100;
                }
            }

            if (st.piecesDestroyedLastMove().size() > 0) {
                for (int p = 0; p < st.piecesDestroyedLastMove().size(); p++) {
                    if (st.piecesDestroyedLastMove()[p].first != coloresJugador[(i+1)%2]) {
                        valoracionJugador += 20;
                    }
                }
            }

            // Si metemos una ficha.
            if (st.isGoalMove() && st.getCurrentPlayerId() == jugador) {
                valoracionJugador += 40;
            }
        }

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

        // Ahora calculo la misma heurística para el oponente.
        double valoracionOponente = 0;
        // Recorro colores de mi oponente.
        for (int i = 0; i < coloresOponente.size(); i++) {
            color c = coloresOponente[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++) {
                // Si ponemos una ficha en una casilla segura.
                if (st.isSafePiece(c, j)) {
                    valoracionOponente += 10;
                }
                // Si nos comen:
                if (st.getBoard().getPiece(c, j).get_box().type == home) { 
                    valoracionOponente -= 50;
                }                
            }            

            // Consigue dado especial.
            if (st.itemAcquired() && st.getCurrentPlayerId() == oponente) {
                if (st.getItemAcquired() == star || st.getItemAcquired() == bullet) {  // dar puntuación a cada objeto.
                    valoracionOponente += 25;
                } else if (st.getItemAcquired() == mega_mushroom || st.getItemAcquired() == red_shell || st.getItemAcquired() == blue_shell) { 
                    valoracionOponente += 15;
                } else {
                    valoracionOponente += 10;
                }
            }

            // Fantasma.
            if (st.isBooMove() && st.getCurrentPlayerId() == oponente) {
                valoracionOponente += 15;
            }

            // Cohete.
            if (st.isBulletMove() && st.getCurrentPlayerId() == oponente) {
                valoracionOponente += 35;
                /*if (st.anyWall(std::get<2>(st.getLastMoves().back()), std::get<3>(st.getLastMoves().back())).size() > 0) {
                    valoracionOponente += 10; // Atraviesa barrera.
                }
                if (st.anyMegaWall(std::get<2>(st.getLastMoves().back()), std::get<3>(st.getLastMoves().back())).size() > 0) {
                    valoracionOponente += 20; // Atraviesa mega-barrera.
                }*/
            }
 
            // Si nos comemos una ficha.
            if(st.isEatingMove() && st.getCurrentPlayerId() == oponente){
                if(st.eatenPiece().first == coloresOponente[(i+1)%2]){
                    // Si nos comemos una ficha de nuestro equipo pero estamos más cerca de ganar con el color que comemos --> Positivo
                    if (st.piecesAtGoal(st.eatenPiece().first) > st.piecesAtGoal(st.getCurrentColor()))
                        valoracionOponente -= 10;
                    else
                        valoracionOponente += 10;
                } else {
                    valoracionOponente += 100;
                }
            }

            if (st.piecesDestroyedLastMove().size() > 0) {
                for (int p = 0; p < st.piecesDestroyedLastMove().size(); p++) {
                    if (st.piecesDestroyedLastMove()[p].first != coloresOponente[(i+1)%2]) {
                        valoracionOponente += 20;
                    }
                }
            }

            // Si metemos una ficha.
            if (st.isGoalMove() && st.getCurrentPlayerId() == oponente) {
                valoracionOponente += 40;
            }
        }
        
        // El valor final de la heurística será la diferencia entre mi valoración y la de mi oponente.
        return valoracionJugador - valoracionOponente;
    }
}

double AIPlayer::Heuristica(const Parchis &st, int jugador) {
    int ganador = st.getWinner();
    int oponente = (jugador + 1) % 2;

    // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
    if (ganador == jugador) {
        return gana;
    } else if (ganador == oponente) {
        return pierde;
    } else {
        // Colores que juega mi jugador y colores del oponente
        vector<color> coloresJugador = st.getPlayerColors(jugador);
        vector<color> coloresOponente = st.getPlayerColors(oponente);

        double valoracionJugador = 0;
        // Recorro colores de mi jugador.
        for (int i = 0; i < coloresJugador.size(); i++) {
            color c = coloresJugador[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++) {
                // Si ponemos una ficha en una casilla segura.
                if (st.isSafePiece(c, j)) {
                    valoracionJugador += 10;
                }
                // Si nos comen:
                if (st.getBoard().getPiece(c, j).get_box().type == home) { //////////////////////// DUDA.
                    valoracionJugador -= 50;
                }               
                // Mayor importancia a las fichas más cercanas a la meta.
                //valoracionJugador += 74 - st.distanceToGoal(c,j);   
            }

            // Si hacemos un movimiento ilegal
            if (st.illegalMove() && st.getCurrentPlayerId() == jugador) {
                valoracionJugador += pierde; 
            }

            // Si pasamos por una trampa. /////////////////////// DUDA
            /*if (st.getLastMoves().size() > 0) {
                Box casillaActual = std::get<3>(st.getLastMoves().back());
                Box casillaInicial = std::get<2>(st.getLastMoves().back());
                vector<BoardTrap> trampas = st.anyTrap(casillaInicial, casillaActual);
                bool hayTrampas = false;
                for (int t = 0; t < trampas.size(); t++) {
                    if (trampas[t].getType() == banana_trap) {
                        hayTrampas = true;
                        t = trampas.size();
                    }
                }
                if (hayTrampas && st.getCurrentPlayerId() == jugador) {
                    valoracionJugador -= 20;
                }
            }**/
            

            // Si usamos un dado especial. (Puede hacer que se utilice champiñón y rayo)
            if (st.isSpecialDice(st.getLastDice()) && st.getCurrentPlayerId() == jugador) {
                valoracionJugador += 5;
            }

            // Estrella.
            if (st.isStarMove() && st.getCurrentPlayerId() == jugador) {
                if (st.piecesDestroyedByStar().size() > 0) {
                    for (int d = 0; d < st.piecesDestroyedByStar().size(); d++) {
                        if(st.piecesDestroyedByStar()[0].first == coloresJugador[(i+1)%2]){
                            valoracionJugador -= 15;
                        } else {
                            valoracionJugador += 30;
                        }
                    }
                }
            }
            

            // MegaChampiñón.
            if (st.isMegaMushroomMove() && st.getCurrentPlayerId() == jugador) {
                if (st.piecesCrushedByMegamushroom().size() > 0) {
                    for (int d = 0; d < st.piecesCrushedByMegamushroom().size(); d++) {
                        if(st.piecesCrushedByMegamushroom()[0].first == coloresJugador[(i+1)%2]){
                            valoracionJugador -= 15;
                        } else {
                            valoracionJugador += 30;
                        }
                    }
                }
            }
            

            // Consigue dado especial.
            if (st.itemAcquired() && st.getCurrentPlayerId() == jugador) {
                if (st.getItemAcquired() == star || st.getItemAcquired() == bullet) {  // dar puntuación a cada objeto.
                    valoracionJugador += 25;
                } else if (st.getItemAcquired() == mega_mushroom || st.getItemAcquired() == red_shell || st.getItemAcquired() == blue_shell) { 
                    valoracionJugador += 15;
                } else {
                    valoracionJugador += 10;
                }
            }

            // Caparazón Azul.
            if (st.isBlueShellMove() && st.getCurrentPlayerId() == jugador) {
                if (st.piecesDestroyedByBlueShell().size() == 2) {
                    if(st.piecesDestroyedByBlueShell()[0].first == coloresJugador[(i+1)%2]){
                        valoracionJugador -= 20;
                    } else {
                        valoracionJugador += 25;
                    }
                    if(st.piecesDestroyedByBlueShell()[1].first == coloresJugador[(i+1)%2]){
                        valoracionJugador -= 20;
                    } else {
                        valoracionJugador += 25;
                    }
                }
                if (st.piecesDestroyedByBlueShell().size() == 1) {
                    if(st.piecesDestroyedByBlueShell()[0].first == coloresJugador[(i+1)%2]){
                        valoracionJugador -= 20;
                    } else {
                        valoracionJugador += 25;
                    }
                }
                if (st.piecesDestroyedByBlueShell().size() == 0) {
                    valoracionJugador -= 15;
                }
            }

            // Fantasma.
            if (st.isBooMove() && st.getCurrentPlayerId() == jugador) {
                valoracionJugador += 5;
            }

            // Cohete.
            if (st.isBulletMove() && st.getCurrentPlayerId() == jugador) {
                valoracionJugador += 25;
                if (st.anyWall(std::get<2>(st.getLastMoves().back()), std::get<3>(st.getLastMoves().back())).size() > 0) {
                    valoracionJugador += 5; // Atraviesa barrera.
                }
                if (st.anyMegaWall(std::get<2>(st.getLastMoves().back()), std::get<3>(st.getLastMoves().back())).size() > 0) {
                    valoracionJugador += 10; // Atraviesa mega-barrera.
                }
            }

            // Bocina.
            if (st.isHornMove() && st.getCurrentPlayerId() == jugador) {
                if (st.piecesDestroyedByHorn().size() > 0) {
                    for (int d = 0; d < st.piecesDestroyedByHorn().size(); d++) {
                        if(st.piecesDestroyedByHorn()[0].first == coloresJugador[(i+1)%2]){
                            valoracionJugador -= 15;
                        } else {
                            valoracionJugador += 30;
                        }
                    }
                } else {
                    valoracionJugador -= 10; // Evitar malgastarlo.
                }
            }

            // Caparazón rojo.
            if (st.isRedShellMove() && st.getCurrentPlayerId() == jugador) {
                if (st.piecesDestroyedByRedShell().size() == 2) {
                    if(st.piecesDestroyedByRedShell()[0].first == coloresJugador[(i+1)%2]){
                        valoracionJugador -= 15;
                    } else {
                        valoracionJugador += 30;
                    }
                    if(st.piecesDestroyedByRedShell()[1].first == coloresJugador[(i+1)%2]){
                        valoracionJugador -= 15;
                    } else {
                        valoracionJugador += 30;
                    }
                }
                if (st.piecesDestroyedByRedShell().size() == 1) {
                    if(st.piecesDestroyedByRedShell()[0].first == coloresJugador[(i+1)%2]){
                        valoracionJugador -= 20;
                    } else {
                        valoracionJugador += 30;
                    }
                }
                if (st.piecesDestroyedByRedShell().size() == 0) {
                    valoracionJugador -= 10;
                }
            }
 
            // Si nos comemos una ficha.
            if(st.isEatingMove() && st.getCurrentPlayerId() == jugador){
                if(st.eatenPiece().first == coloresJugador[(i+1)%2]){
                    // Si nos comemos una ficha de nuestro equipo pero estamos más cerca de ganar con el color que comemos --> Positivo
                    if (st.piecesAtGoal(st.eatenPiece().first) > st.piecesAtGoal(st.getCurrentColor()))
                        valoracionJugador -= 10;
                    else
                        valoracionJugador += 10;
                } else {
                    valoracionJugador += 35;
                }
            }

            // Si metemos una ficha.
            if (st.isGoalMove() && st.getCurrentPlayerId() == jugador) {
                valoracionJugador += 25;
            }
        }
        
        // Ahora calculo la misma heurística para el oponente.
        double valoracionOponente = 0;
        for (int i = 0; i < coloresOponente.size(); i++) {
            color c = coloresOponente[i];
            for (int j = 0; j < num_pieces; j++) {
                if (st.isSafePiece(c, j)) {
                    valoracionOponente += 10;
                }
                if (st.getBoard().getPiece(c, j).get_box().type == home) {
                    valoracionOponente -= 40;
                }
                
               // valoracionOponente += 74 - st.distanceToGoal(c,j);
            }

            if (st.illegalMove() && st.getCurrentPlayerId() == oponente) {
                valoracionOponente += pierde; 
            }

           /* Box casillaActual = std::get<3>(st.getLastMoves().back());
            Box casillaInicial = std::get<2>(st.getLastMoves().back());
            vector<BoardTrap> trampas = st.anyTrap(casillaInicial, casillaActual);
            bool hayTrampas = false;
            for (int t = 0; t < trampas.size(); t++) {
                if (trampas[t].getType() == banana_trap) {
                    hayTrampas = true;
                    t = trampas.size();
                }
            }
            if (hayTrampas &&  st.getCurrentPlayerId() == oponente) {
                valoracionOponente -= 20;
            }*/

            // Si usamos un dado especial. (Puede hacer que se utilice champiñón y rayo)
            if (st.isSpecialDice(st.getLastDice()) && st.getCurrentPlayerId() == oponente) {
                valoracionOponente += 5;
            }

            // Estrella.
            if (st.isStarMove() && st.getCurrentPlayerId() == oponente) {
                if (st.piecesDestroyedByStar().size() > 0) {
                    for (int d = 0; d < st.piecesDestroyedByStar().size(); d++) {
                        if(st.piecesDestroyedByStar()[0].first == coloresOponente[(i+1)%2]){
                            valoracionOponente -= 15;
                        } else {
                            valoracionOponente += 30;
                        }
                    }
                }
            }            

            // MegaChampiñón.
            if (st.isMegaMushroomMove() && st.getCurrentPlayerId() == oponente) {
                if (st.piecesCrushedByMegamushroom().size() > 0) {
                    for (int d = 0; d < st.piecesCrushedByMegamushroom().size(); d++) {
                        if(st.piecesCrushedByMegamushroom()[0].first == coloresOponente[(i+1)%2]){
                            valoracionOponente -= 15;
                        } else {
                            valoracionOponente += 30;
                        }
                    }
                }
            }
            

            // Consigue dado especial.
            if (st.itemAcquired() && st.getCurrentPlayerId() == oponente) {
                if (st.getItemAcquired() == star || st.getItemAcquired() == bullet) {  // dar puntuación a cada objeto.
                    valoracionOponente += 25;
                } else if (st.getItemAcquired() == mega_mushroom || st.getItemAcquired() == red_shell || st.getItemAcquired() == blue_shell) { 
                    valoracionOponente += 15;
                } else {
                    valoracionOponente += 10;
                }
            }

            // Caparazón Azul.
            if (st.isBlueShellMove() && st.getCurrentPlayerId() == oponente) {
                if (st.piecesDestroyedByBlueShell().size() == 2) {
                    if(st.piecesDestroyedByBlueShell()[0].first == coloresOponente[(i+1)%2]){
                        valoracionOponente -= 20;
                    } else {
                        valoracionOponente += 35;
                    }
                    if(st.piecesDestroyedByBlueShell()[1].first == coloresOponente[(i+1)%2]){
                        valoracionOponente -= 20;
                    } else {
                        valoracionOponente += 35;
                    }
                }
                if (st.piecesDestroyedByBlueShell().size() == 1) {
                    if(st.piecesDestroyedByBlueShell()[0].first == coloresOponente[(i+1)%2]){
                        valoracionOponente -= 20;
                    } else {
                        valoracionOponente += 35;
                    }
                }
                if (st.piecesDestroyedByBlueShell().size() == 0) {
                    valoracionOponente -= 15;
                }
            }

            // Fantasma.
            if (st.isBooMove() && st.getCurrentPlayerId() == oponente) {
                valoracionOponente += 5;
            }

            // Cohete.
            if (st.isBulletMove() && st.getCurrentPlayerId() == oponente) {
                valoracionOponente += 25;
                if (st.anyWall(std::get<2>(st.getLastMoves().back()), std::get<3>(st.getLastMoves().back())).size() > 0) {
                    valoracionOponente += 5; // Atraviesa barrera.
                }
                if (st.anyMegaWall(std::get<2>(st.getLastMoves().back()), std::get<3>(st.getLastMoves().back())).size() > 0) {
                    valoracionOponente += 10; // Atraviesa mega-barrera.
                }
            }

            // Bocina.
            if (st.isHornMove() && st.getCurrentPlayerId() == oponente) {
                if (st.piecesDestroyedByHorn().size() > 0) {
                    for (int d = 0; d < st.piecesDestroyedByHorn().size(); d++) {
                        if(st.piecesDestroyedByHorn()[0].first == coloresOponente[(i+1)%2]){
                            valoracionOponente -= 15;
                        } else {
                            valoracionOponente += 30;
                        }
                    }
                } else {
                    valoracionOponente -= 10; // Evitar malgastarlo.
                }
            }

            // Caparazón rojo.
            if (st.isRedShellMove() && st.getCurrentPlayerId() == oponente) {
                if (st.piecesDestroyedByRedShell().size() == 2) {
                    if(st.piecesDestroyedByRedShell()[0].first == coloresOponente[(i+1)%2]){
                        valoracionOponente -= 15;
                    } else {
                        valoracionOponente += 30;
                    }
                    if(st.piecesDestroyedByRedShell()[1].first == coloresOponente[(i+1)%2]){
                        valoracionOponente -= 15;
                    } else {
                        valoracionOponente += 30;
                    }
                }
                if (st.piecesDestroyedByRedShell().size() == 1) {
                    if(st.piecesDestroyedByRedShell()[0].first == coloresOponente[(i+1)%2]){
                        valoracionOponente -= 20;
                    } else {
                        valoracionOponente += 30;
                    }
                }
                if (st.piecesDestroyedByRedShell().size() == 0) {
                    valoracionOponente -= 10;
                }
            }

            if(st.isEatingMove() && st.getCurrentPlayerId() == oponente){
                if(st.eatenPiece().first == coloresOponente[(i+1)%2]){
                    if (st.piecesAtGoal(st.eatenPiece().first) > st.piecesAtGoal(st.getCurrentColor()))
                        valoracionOponente -= 10;
                    else
                        valoracionOponente += 10;
                }else{
                    valoracionOponente += 35;
                }
            }
            if (st.isGoalMove() && st.getCurrentPlayerId() == oponente) {
                valoracionOponente += 25;
            }
        }
        
        // El valor final de la heurística será la diferencia entre mi valoración y la de mi oponente.
        return valoracionJugador - valoracionOponente;
    }
}

double AIPlayer::Poda_AlfaBeta(const Parchis &actual, int jugador, int profundidad, int profundidadMaxima, color &c_piece, int &id_piece, int &dice, double alpha, double beta, double (*Heuristica)(const Parchis &, int)) const {
    // Si alcanzamos la profundidad máxima (6) o llegamos a una situación de victoria evaluamos el st actual con la heurística.
    if (profundidad == profundidadMaxima || actual.gameOver()) {
        return Heuristica(actual, jugador);
    }

    ParchisBros hijos = actual.getChildren();

    if (jugador == actual.getCurrentPlayerId()) { // Si estamos en un nodo MAX.
        for(ParchisBros::Iterator it = hijos.begin(); it != hijos.end(); ++it){
            Parchis siguiente_hijo = *it;
            double eval = Poda_AlfaBeta(siguiente_hijo, jugador, profundidad + 1, profundidadMaxima, c_piece, id_piece, dice, alpha, beta, Heuristica);

            if(eval > alpha){
                alpha = eval; //Ya que queremos coger el valor MAXIMO de cada nodo
                if (profundidad == 0) {
                    c_piece = it.getMovedColor(); // Guardo color de la ficha movida.
                    id_piece = it.getMovedPieceId(); // Guardo id de la ficha movida.
                    dice = it.getMovedDiceValue(); // Guardo número de dado movido.                
                }
            }

            if (beta <= alpha) {
                break;
            } // En este caso no seguimos evaluando dicha rama.

        }

        return alpha;
    } else { // Estamos en un nodo MIN.
        for(ParchisBros::Iterator it = hijos.begin(); it != hijos.end(); ++it){    
            Parchis siguiente_hijo = *it;        
            double eval = Poda_AlfaBeta(siguiente_hijo, jugador, profundidad + 1, profundidadMaxima, c_piece, id_piece, dice, alpha, beta, Heuristica);
            
            if(eval < beta){
                beta = eval;
            }
            
            if(alpha >= beta){
                break;
            }
        }
        
        return beta;
    }
}

double AIPlayer::Heuristica4(const Parchis &st, int jugador) {
    int ganador = st.getWinner();
    int oponente = (jugador+1) % 2;

    // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
    if (ganador == jugador) {
        return gana;
    } else if (ganador == oponente) {
        return pierde;
    } else {
        // Colores que juega mi jugador y colores del oponente
        vector<color> coloresJugador = st.getPlayerColors(jugador);
        vector<color> coloresOponente = st.getPlayerColors(oponente);

        // Recorro todas las fichas de mi jugador
        int valoracionJugador = 0;
        // Recorro colores de mi jugador.
        for (int i = 0; i < coloresJugador.size(); i++) {
            color c = coloresJugador[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++) {
                // Valoro positivamente que la ficha esté en casilla segura o meta.
                if (st.isSafePiece(c, j)) {
                    valoracionJugador++;
                } else if (st.getBoard().getPiece(c, j).get_box().type == goal) {
                    valoracionJugador += 5;
                } else if (st.getBoard().getPiece(c, j).get_box().type == home) { // Nos comen.
                    valoracionJugador -= 3;
                }
            }

            // Si nos comemos una ficha.
            if(st.isEatingMove() && st.getCurrentPlayerId() == jugador){
                if(st.eatenPiece().first != coloresJugador[(i+1)%2]){
                    valoracionJugador += 6;
                } else {
                    valoracionJugador -= 2;
                }
            }

            if (st.piecesDestroyedLastMove().size() > 0 && st.getCurrentPlayerId() == jugador) {
                for (int p = 0; p < st.piecesDestroyedLastMove().size(); p++) {
                    if (st.piecesDestroyedLastMove()[p].first != coloresJugador[(i+1)%2]) {
                        valoracionJugador+=2;
                    }
                }
            }
        }

        // Consigue dado especial.
        if (st.itemAcquired() && st.getCurrentPlayerId() == jugador) {
            if (st.getItemAcquired() == star || st.getItemAcquired() == bullet || st.getItemAcquired() == horn) {  // dar puntuación a cada objeto.
                valoracionJugador += 2;
            } else {
                valoracionJugador += 1;
            }
        }

        // Recorro todas las fichas del oponente
        int valoracionOponente = 0;
        // Recorro colores del oponente.
        for (int i = 0; i < coloresOponente.size(); i++) {
            color c = coloresOponente[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++) {
                if (st.isSafePiece(c, j)) {
                    // Valoro negativamente que la ficha esté en casilla segura o meta.
                    valoracionOponente++;
                } else if (st.getBoard().getPiece(c, j).get_box().type == goal) {
                    valoracionOponente += 5;
                } else if (st.getBoard().getPiece(c, j).get_box().type == home) {
                    valoracionOponente -= 3;
                }
            }

            // Si nos comemos una ficha.
            if(st.isEatingMove() && st.getCurrentPlayerId() == oponente){
                if(st.eatenPiece().first != coloresOponente[(i+1)%2]){
                    valoracionOponente += 6;
                } else {
                    valoracionOponente -= 2;
                }
            }

            if (st.piecesDestroyedLastMove().size() > 0 && st.getCurrentPlayerId() == oponente) {
                for (int p = 0; p < st.piecesDestroyedLastMove().size(); p++) {
                    if (st.piecesDestroyedLastMove()[p].first != coloresOponente[(i+1)%2]) {
                        valoracionOponente+=2;
                    }
                }
            }
        }

        // Consigue dado especial.
        if (st.itemAcquired() && st.getCurrentPlayerId() == oponente) {
            if (st.getItemAcquired() == star || st.getItemAcquired() == bullet || st.getItemAcquired() == horn) {  // dar puntuación a cada objeto.
                valoracionOponente += 2;
            } else {
                valoracionOponente += 1;
            }
        }

        // Devuelvo la puntuación de mi jugador menos la puntuación del oponente.
        return valoracionJugador - valoracionOponente;
    }
}


void AIPlayer::think(color & c_piece, int & id_piece, int & dice) const {
    /*switch(id){
        case 0:
            thinkAleatorio(c_piece, id_piece, dice);
        break;
        case 1:
            thinkAleatorioMasInteligente(c_piece, id_piece, dice);
        break;
        case 2:
            thinkFichaMasAdelantada(c_piece, id_piece, dice);
        break;
        case 3:
            thinkMejorOpcion(c_piece, id_piece, dice);
        break;
    }*/

    double valor; // Almacena el valor con el que se etiqueta el st tras el proceso de busqueda.
    double alpha = menosinf, beta = masinf; // Cotas iniciales de la poda AlfaBeta
    cout << "Valor MiniMax: " << valor << "  Accion: " << str(c_piece) << " " << id_piece << " " << dice << endl;

    // Si quiero poder manejar varias heurísticas, puedo usar la variable id del agente para usar una u otra.
    switch(id){
        case 0:
            valor = Poda_AlfaBeta(*actual, jugador, 0, PROFUNDIDAD_ALFABETA, c_piece, id_piece, dice, alpha, beta, ValoracionTest);
            break;
        case 1:
            valor = Poda_AlfaBeta(*actual, jugador, 0, PROFUNDIDAD_ALFABETA, c_piece, id_piece, dice, alpha, beta, Heuristica4);
            break;
        case 2:
            valor = Poda_AlfaBeta(*actual, jugador, 0, PROFUNDIDAD_ALFABETA, c_piece, id_piece, dice, alpha, beta, Heuristica3);
            break;
    }
    cout << "Valor MiniMax: " << valor << "  Accion: " << str(c_piece) << " " << id_piece << " " << dice << endl;


}




double AIPlayer::ValoracionTest(const Parchis &st, int jugador)
{
    // Heurística de prueba proporcionada para validar el funcionamiento del algoritmo de búsqueda.


    int ganador = st.getWinner();
    int oponente = (jugador+1) % 2;

    // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
    if (ganador == jugador)
    {
        return gana;
    }
    else if (ganador == oponente)
    {
        return pierde;
    }
    else
    {
        // Colores que juega mi jugador y colores del oponente
        vector<color> coloresJugador = st.getPlayerColors(jugador);
        vector<color> coloresOponente = st.getPlayerColors(oponente);

        // Recorro todas las fichas de mi jugador
        int valoracionJugador = 0;
        // Recorro colores de mi jugador.
        for (int i = 0; i < coloresJugador.size(); i++)
        {
            color c = coloresJugador[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++)
            {
                // Valoro positivamente que la ficha esté en casilla segura o meta.
                if (st.isSafePiece(c, j))
                {
                    valoracionJugador++;
                }
                else if (st.getBoard().getPiece(c, j).get_box().type == goal)
                {
                    valoracionJugador += 5;
                }
            }
        }

        // Recorro todas las fichas del oponente
        int valoracionOponente = 0;
        // Recorro colores del oponente.
        for (int i = 0; i < coloresOponente.size(); i++)
        {
            color c = coloresOponente[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++)
            {
                if (st.isSafePiece(c, j))
                {
                    // Valoro negativamente que la ficha esté en casilla segura o meta.
                    valoracionOponente++;
                }
                else if (st.getBoard().getPiece(c, j).get_box().type == goal)
                {
                    valoracionOponente += 5;
                }
            }
        }

        // Devuelvo la puntuación de mi jugador menos la puntuación del oponente.
        return valoracionJugador - valoracionOponente;
    }
}