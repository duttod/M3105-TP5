#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(nullptr) {
}

void Interpreteur::analyse() {
    m_arbre = programme(); // on lance l'analyse de la première règle
}

void Interpreteur::tester(const string & symboleAttendu) const throw (SyntaxeException) {
    // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
    static char messageWhat[256];
    if (m_lecteur.getSymbole() != symboleAttendu) {
        sprintf(messageWhat,
                "Ligne %d, Colonne %d - Erreur de syntaxe - Symbole attendu : %s - Symbole trouvé : %s",
                m_lecteur.getLigne(), m_lecteur.getColonne(),
                symboleAttendu.c_str(), m_lecteur.getSymbole().getChaine().c_str());
        throw SyntaxeException(messageWhat);
    }
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException) {
    // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
    tester(symboleAttendu);
    m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const throw (SyntaxeException) {
    // Lève une exception contenant le message et le symbole courant trouvé
    // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
    static char messageWhat[256];
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
}

Noeud* Interpreteur::programme() {
    // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    testerEtAvancer("procedure");
    testerEtAvancer("principale");
    testerEtAvancer("(");
    testerEtAvancer(")");
    Noeud* sequence = seqInst();
    testerEtAvancer("finproc");
    tester("<FINDEFICHIER>");
    return sequence;
}

Noeud* Interpreteur::seqInst() {
    // <seqInst> ::= <inst> { <inst> }
    NoeudSeqInst* sequence = new NoeudSeqInst();
    do {
        sequence->ajoute(inst());
    } while (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "si" || m_lecteur.getSymbole() == "tantque" || m_lecteur.getSymbole() == "pour" || m_lecteur.getSymbole() == "ecrire" || m_lecteur.getSymbole() == "lire" || m_lecteur.getSymbole() == "repeter");
    // Tant que le symbole courant est un début possible d'instruction...
    // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
    return sequence;
}

Noeud* Interpreteur::inst() {
    // <inst> ::= <affectation>  ; | <instSi>
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        Noeud *affect = affectation();
        testerEtAvancer(";");
        return affect;
    } else if (m_lecteur.getSymbole() == "si")
        return instSiRiche();
        // Compléter les alternatives chaque fois qu'on rajoute une nouvelle instruction
    else if (m_lecteur.getSymbole() == "tantque")
        return instTQ();
    else if (m_lecteur.getSymbole() == "pour")
        return instPour();
    else if (m_lecteur.getSymbole() == "ecrire")
        return instEcrire();
    else if (m_lecteur.getSymbole() == "lire")
        return instLire();
    else if (m_lecteur.getSymbole() == "repeter")
        return repeter();
    else erreur("Instruction incorrecte");
}

Noeud* Interpreteur::affectation() {
    // <affectation> ::= <variable> = <expression> 
    tester("<VARIABLE>");
    Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
    m_lecteur.avancer();
    testerEtAvancer("=");
    Noeud* exp = expression(); // On mémorise l'expression trouvée
    return new NoeudAffectation(var, exp); // On renvoie un noeud affectation
}

Noeud* Interpreteur::expression() {
    // <expression> ::= <facteur> { <opBinaire> <facteur> }
    //  <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
    Noeud* fact = facteur();
    while (m_lecteur.getSymbole() == "+" || m_lecteur.getSymbole() == "-" ||
            m_lecteur.getSymbole() == "*" || m_lecteur.getSymbole() == "/" ||
            m_lecteur.getSymbole() == "<" || m_lecteur.getSymbole() == "<=" ||
            m_lecteur.getSymbole() == ">" || m_lecteur.getSymbole() == ">=" ||
            m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" ||
            m_lecteur.getSymbole() == "et" || m_lecteur.getSymbole() == "ou") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* factDroit = facteur(); // On mémorise l'opérande droit
        fact = new NoeudOperateurBinaire(operateur, fact, factDroit); // Et on construuit un noeud opérateur binaire
    }
    return fact; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::facteur() {
    // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
    Noeud* fact = nullptr;
    if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
        fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
        m_lecteur.avancer();
    } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), facteur());
    } else if (m_lecteur.getSymbole() == "non") { // non <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustractin binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("non"), facteur(), nullptr);
    } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
        m_lecteur.avancer();
        fact = expression();
        testerEtAvancer(")");
    } else
        erreur("Facteur incorrect");
    return fact;
}

Noeud* Interpreteur::instSi() {
    // <instSi> ::= si ( <expression> ) <seqInst> finsi
    testerEtAvancer("si");
    testerEtAvancer("(");
    Noeud* condition = expression(); // On mémorise la condition
    testerEtAvancer(")");
    Noeud* sequence = seqInst(); // On mémorise la séquence d'instruction
    testerEtAvancer("finsi");
    return new NoeudInstSi(condition, sequence); // Et on renvoie un noeud Instruction Si
}

Noeud* Interpreteur::instTQ() {
    // <instTantQue> ::= tantque ( <expression> ) <seqInst> fintantque 
    testerEtAvancer("tantque");
    testerEtAvancer("(");
    Noeud * cond = expression();
    testerEtAvancer(")");
    Noeud * sequence = seqInst();
    testerEtAvancer("fintantque");
    return new NoeudInstTQ(cond, sequence);
}

Noeud* Interpreteur::instPour() {
    // <instPour>    ::= pour ([<affectation>] ; <expression> ; [<affectation>]) <seqInst> finpour
    testerEtAvancer("pour");
    testerEtAvancer("(");
    Noeud * aff1 = nullptr;
    Noeud * aff2 = nullptr;
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        aff1 = affectation();
    }
    testerEtAvancer(";");
    Noeud * cond = expression();
    testerEtAvancer(";");
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        aff2 = affectation();
    }
    testerEtAvancer(")");
    Noeud * sequence = seqInst();
    testerEtAvancer("finpour");
    return new NoeudInstPour(aff1, cond, aff2, sequence);
}

Noeud* Interpreteur::instEcrire() {
    //<instEcrire>  ::= ecrire( <expression> | <chaine> {, <expression> | <chaine> })
    testerEtAvancer("ecrire");
    testerEtAvancer("(");
    Noeud* noeud1 = nullptr; //Variable qui serviras a l'ajout de tout les noeuds dans le vecteur

    if (m_lecteur.getSymbole() == "<CHAINE>") {
        noeud1 = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
        m_lecteur.avancer();

    } else { // sinon c'est une expression
        noeud1 = expression();
    }
    std::vector<Noeud*> v;
    v.push_back(noeud1);

    while (m_lecteur.getSymbole() == ",") { // on regarde si il y a d'autres choses à écrire
        m_lecteur.avancer();
        if (m_lecteur.getSymbole() == "<CHAINE>") {
            noeud1 = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
            m_lecteur.avancer();
            v.push_back(noeud1);
        } else {
            noeud1 = expression();
            v.push_back(noeud1);
        }
    }
    testerEtAvancer(")");
    testerEtAvancer(";");

    return new NoeudInstEcrire(v);

}

Noeud* Interpreteur::instLire() {

    testerEtAvancer("lire");
    testerEtAvancer("(");
    Noeud* noeud1 = nullptr;
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        noeud1 = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable à la table
        m_lecteur.avancer();
    }
    std::vector<Noeud*> v;
    v.push_back(noeud1);
    
    while (m_lecteur.getSymbole() == ",") { // on regarde si il y a d'autres choses à lire
        m_lecteur.avancer();
        if (m_lecteur.getSymbole() == "<VARIABLE>") {
            noeud1 = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variableà la table
            m_lecteur.avancer();
            v.push_back(noeud1);
        }
    }
    testerEtAvancer(")");
    testerEtAvancer(";");

    return new NoeudInstLire(v);
}

Noeud* Interpreteur::instSiRiche() {
    vector<Noeud*> conditions;
    vector<Noeud*> sequences;
    testerEtAvancer("si");
    testerEtAvancer("(");
    Noeud* condition = expression(); // On mémorise la condition
    conditions.push_back(condition);
    testerEtAvancer(")");
    Noeud* sequence = seqInst(); // On mémorise la séquence d'instruction
    sequences.push_back(sequence);
    try {
        do {
            testerEtAvancer("sinonsi");
            testerEtAvancer("(");
            Noeud* condition = expression(); // On mémorise la condition
            conditions.push_back(condition);
            testerEtAvancer(")");
            Noeud* sequence = seqInst(); // On mémorise la séquence d'instruction
            sequences.push_back(sequence);
        } while (true);
    } catch (exception e) {
    }
    Noeud* sequencesinon = nullptr;
    try{
        testerEtAvancer("sinon");
        sequencesinon = seqInst();
    }catch(exception e){        
    }
    testerEtAvancer("finsi");
    return new NoeudSiRiche(conditions, sequences, sequencesinon); // Et on renvoie un noeud Instruction Si    
} //     <siRiche> ::= si ( <expression> ) <seqInst> {sinon si ( <expression> ) <seqInst> } [sinon <seqInst>] finsi

Noeud* Interpreteur::repeter() {
    //    <repeter> ::= repeter <seqInst> jusqua ( <expression> )
    testerEtAvancer("repeter");
    Noeud* sequence = seqInst();
    testerEtAvancer("jusqua");
    testerEtAvancer("(");
    Noeud* condition = expression(); // On mémorise la condition
    testerEtAvancer(")");
    return new NoeudRepeter(condition, sequence);
}

void Interpreteur::traduitEnCPP(ostream & cout,unsigned int indentation)const{
    cout << "\n\n// ===================TRADUCTION EN C++================\n"<<endl;
    cout << setw(4*indentation)<<""<<"#include <iostream>"<<endl; // affiche tout le nécessaire pour que le code généré puisse compiler
    cout << setw(4*indentation)<<""<<"using namespace std;"<<endl;
    cout << setw(4*indentation)<<""<<"\nint main() {"<< endl;
  
  for (int i = 0 ; i < m_table.getTaille() ; i++){ // pour écrire int i; int j; etc...
      if (m_table[i]=="<VARIABLE>"){
          cout << setw(4* (indentation+1)) << "" << "int " << m_table[i].getChaine()<<";"<<endl;
      }
  }
  getArbre()->traduitEnCPP(cout,indentation+1);// lance l'opération traduitEnCPP sur la racine
  cout <<"\n"<< setw(4*(indentation+1))<<""<<"return 0;"<< endl ; 
  cout << setw(4*indentation)<<"}" << endl ; // Fin d’un programme C++
} 

