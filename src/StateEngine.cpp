#include "StateEngine.hpp"
#include <algorithm>




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


        if (dist >= 10){

            int hpAdd = dist/10;

            for (Pokemon &bicho : treinador.party){

                if (bicho.status == PokemonStatus::Consciente){
                    bicho.hp = bicho.hp + hpAdd;

                    if (bicho.max_hp < bicho.hp){
                        bicho.hp = bicho.max_hp;
                    }
                }
            }
        }
    }






    void andandoIndisponivelReducao(Treinador &treinador, int dist){


        for (Pokemon &bicho : treinador.party){


            if (bicho.status == PokemonStatus::Inconsciente || bicho.status == PokemonStatus::No_PMC){


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
                peso = m.peso;
                break;
            }

        }

        if (!tem){

            return false;
        }

        treinador.no_atual = destino;
        AvancarTempo(estado, treinador, peso); //TRABALHAR NISSO DPS

        return true;

    }




    Treinador* encontrarTreinadorPorId(GameState &estado, int id){

        for (Treinador &t : estado.treinadores){

            if (t.id == id){
                return &t;
            }
        }

        return nullptr;
    }






    bool usarErva(Treinador& treinador) {


        if (treinador.ervas <= 0){
            return false;
        }


        for (Pokemon &p : treinador.party){

            if (p.status == PokemonStatus::Consciente){

                p.hp = p.hp + 10;

                if (p.hp > p.max_hp){
                    p.hp = p.max_hp;
                }
            }
            else if (p.status == PokemonStatus::Inconsciente){
                return false;
            }
        }

        treinador.ervas--;
        return true;

    }






    bool podePegarOvo(const Treinador& treinador) {


        int total = treinador.party.size() + treinador.ovos.size();

        return total < MAX_UNIDADES;
    }







    int criarJogador(GameState &estado, std::string &nomenovo, std::vector<int> &especies){


        Treinador jogador;

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

            if (esp < 0 || esp >= estado.catalogo_especies.size()){
                
                continue;
            }

            Pokemon p = estado.catalogo_especies[esp];

            p.no_atual = jogador.no_atual;


            jogador.party.push_back(p);
        }

        estado.treinadores.push_back(jogador);
        estado.jogador_id = jogador.id;


        estado.nos[estado.laboratorio].treinadores.push_back(jogador.id);

        return jogador.id;
        

    }






}