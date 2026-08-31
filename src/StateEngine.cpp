#include "StateEngine.hpp"
#include <algorithm>
#include <queue>

#include "RNG.hpp"




namespace StateEngine{



    bool verificar_zonasegura(const GameState& estado, int no){


        if (no == estado.laboratorio){
            return true;
        }

        for (int pmc : estado.pmc){
            if (pmc == no){
                return true;
            }
        }

        return false;
    }





    bool equipeCheia(const Treinador& treinador){

        if (treinador.party.size() >= 6){
            return true;
        }
        else {
            return false;
        }

    }




    bool enviarParaCarvalho(Treinador &treinador, int indicepokemon){

        //indicepokemon = num do pokemon na party

        if (indicepokemon < 0 || indicepokemon >= (int)treinador.party.size()){
        
            return false;

        }

        treinador.pc_professor.push_back(treinador.party[indicepokemon]);

        treinador.party.erase(treinador.party.begin() + indicepokemon);

        return true;

    }



    void andandoRecuperarHP(Treinador &treinador, int dist){
        // pokemons conscientes NAO recuperam HP passivamente ao caminhar,
        // independente da vida. A recuperacao de HP se da apenas via
        // Centro Pokemon, itens ou ervas.
        (void)treinador;
        (void)dist;
    }






    void andandoIndisponivelReducao(Treinador &treinador, int dist){


        for (Pokemon &bicho : treinador.party){


            // pokemon inconsciente se recupera passivamente ao caminhar;
            // ja o muito machucado (No_PMC) so se cura no Centro Pokemon
            if (bicho.status == PokemonStatus::Inconsciente){


                bicho.tempo_recuperacao = bicho.tempo_recuperacao - dist;

                if (bicho.tempo_recuperacao <= 0){
                    bicho.status = PokemonStatus::Consciente;
                    bicho.tempo_recuperacao = 0;
                    bicho.hp = bicho.max_hp;
                }
            }
        }

    }







    void andandoChocarOvos (Treinador &treinador, int dist){

        //choca os ovos que ainda não chocraram
        for (Pokemon &ovo : treinador.ovos){

            if(ovo.status != PokemonStatus::Ovo){
                continue;
            }

            ovo.dist_ovo = ovo.dist_ovo + dist;

            if (ovo.dist_ovo >= 100){
                ovo.status = PokemonStatus::Consciente; //nasceu

                ovo.xp = 0;
                ovo.dist_ovo = 0;
            }

        }


        //traz os chocados pro lugar certo
        int i = 0;
        while (i < (int)treinador.ovos.size()) {

            if (treinador.ovos[i].status == PokemonStatus::Consciente) {
                
                if (equipeCheia(treinador) == false) {

                    treinador.party.push_back(treinador.ovos[i]);  
                } 
                 
                else {

                    treinador.pc_professor.push_back(treinador.ovos[i]);
                }
 
                treinador.ovos.erase(treinador.ovos.begin() + i);
            
            }
        
            else {

                 i++;
            
            }

    
        }

    }



    void andandoGanharXP(Treinador& treinador, int dist) {

        for (Pokemon& p : treinador.party) {

            p.dist_xp = p.dist_xp + dist;
            int ganho = p.dist_xp / DIST_XP;

            if (ganho > 0) {

                p.xp = p.xp + ganho;
                p.dist_xp = p.dist_xp - ganho * DIST_XP;
            
            }
        }

    }

    void andandoReporSelvagens(GameState& estado, int dist) {

        for (Pokemon& s : estado.selvagens) {

            if (s.status == PokemonStatus::No_PMC && s.tempo_recuperacao > 0) {

                s.tempo_recuperacao = s.tempo_recuperacao - dist;

                if (s.tempo_recuperacao <= 0) {

                    s.tempo_recuperacao = 0;
                    s.status = PokemonStatus::Consciente;
                    s.hp = s.max_hp;

                    // devolve o selvagem a lista de selvagens do no, para
                    // manter os dados do mapa coerentes
                    if (s.no_atual >= 0 &&
                        s.no_atual < (int)estado.nos.size()) {
                        auto& lista =
                            estado.nos[s.no_atual].pokemons_selvagens;
                        bool ja = false;
                        for (int id : lista)
                            if (id == s.id) { ja = true; break; }
                        if (!ja) lista.push_back(s.id);
                    }

                }
            }
        }

    }





    Treinador* encontrarTreinadorPorId(GameState &estado, int id){

        for (Treinador &t : estado.treinadores){

            if (t.id == id){
                return &t;
            }
        }

        return nullptr;
    }





    std::vector<int> acharQuantidadePontos(GameState &estado, int raiz){


        int nostotal = estado.nos.size();

        std::vector<int> distancia(nostotal, -1);


        if (raiz < 0 || raiz >= nostotal){

            return distancia;

            
        }

        std::queue<int> fila;

        distancia[raiz] = 0;



        fila.push(raiz);
        while (fila.empty() == false){

            int u = fila.front();
            fila.pop();
            for (Arestas &a : estado.nos[u].vizinhos){

                if (distancia[a.para] == -1){

                    distancia[a.para] = distancia[u] + 1;
                    fila.push(a.para);
                }
            }
        }




        return distancia;

    }






    bool usarErva(Treinador& treinador) {


        if (treinador.ervas <= 0){
            return false;
        }


        bool algumConsciente = false;
        for (Pokemon &p : treinador.party){

            if (p.status == PokemonStatus::Consciente){
                algumConsciente = true;
                p.hp = p.hp + 10;

                if (p.hp > p.max_hp){
                    p.hp = p.max_hp;
                }
            }
        }

        // se nao ha nenhum consciente para receber o remedio, a erva nao e usada
        if (!algumConsciente) return false;

        treinador.ervas--;
        return true;

    }






    bool podePegarOvo(const Treinador& treinador) {


        int total = treinador.party.size() + treinador.ovos.size();

        return total < MAX_UNIDADES;
    }





    bool podeBatalhar(Treinador &treinador){


        int cont = 0;

        for (Pokemon &p : treinador.party){

            if (p.status == PokemonStatus::Consciente){
                cont++;
            }
        }

        if (cont >= 1){
            return true;
        }

        return false;

    }




    bool podeInscrever(GameState &estado, Treinador &treinador){


        if (treinador.no_atual != estado.estadio){
            return false;
        }

        // nao da pra inscrever se o prazo ja expirou
        if (prazoExpirado(estado)){
            return false;
        }

        int insigniasnec = std::min(8, (int)estado.ginasios.size());

        if (treinador.insignias >= insigniasnec){
            return true;
        }

        return false;

    }




    bool prazoExpirado(GameState &estado){


        if (estado.tempo_decorrido > estado.tempo_limite){
            return true;
        }
        
        return false;
    }







    int criarJogador(GameState &estado, const std::string &nomenovo, const std::vector<int> &especies){


        Treinador jogador;

        // item inicial definido pelas constantes (nao depende do default da struct)
        jogador.pokebolas = POKEBOLAS_INICIAIS;

        int idprox = 0;

        for (Treinador &t : estado.treinadores){

            if (t.id + 1 > idprox){

                idprox = t.id + 1;
            }
        }


        jogador.nome = nomenovo;
        jogador.id = idprox;

        jogador.no_atual = estado.laboratorio;



        for (int esp : especies){

            if (esp < 0 || esp >= (int)estado.catalogo_especies.size()){
                
                continue;
            }

            Pokemon p = estado.catalogo_especies[esp];

            p.no_atual = jogador.no_atual;

            // requisito: valores iniciais de ap/dp escolhidos aleatoriamente
            p.ap_base = RNG::aleatorio(12, 18);
            p.dp_base = RNG::aleatorio(12, 18);

            jogador.party.push_back(p);
        }

        estado.treinadores.push_back(jogador);
        estado.jogador_id = jogador.id;


        estado.nos[estado.laboratorio].treinadores.push_back(jogador.id);

        return jogador.id;
        

    }





    void AvancarTempo(GameState &estado, Treinador &treinador, int dist){

        estado.tempo_decorrido = estado.tempo_decorrido + dist;

        andandoGanharXP(treinador, dist);

        andandoChocarOvos(treinador, dist);

        andandoRecuperarHP(treinador, dist);

        andandoIndisponivelReducao(treinador, dist);

        andandoReporSelvagens(estado, dist);

    }





    bool moverTreinador(GameState &estado, Treinador& treinador, int destino){

        int atual = treinador.no_atual;
        int total = estado.nos.size();
        int peso = 0;
        bool tem = false;


        if (atual < 0 || atual >= total || destino < 0 || destino >= total){
            return false;
        }

        No &Noatual = estado.nos[atual];


        for (Arestas &m : Noatual.vizinhos){

            if (m.para == destino){
                peso = m.peso;
                tem = true;
                break;
            }

        }

        if (!tem){

            return false;
        }

        treinador.no_atual = destino;
        AvancarTempo(estado, treinador, peso);

        return true;

    }





}