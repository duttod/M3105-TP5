#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"
#include <typeinfo>

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}

int NoeudSeqInst::executer() {
    for (unsigned int i = 0; i < m_instructions.size(); i++)
        m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
    if (instruction != nullptr) m_instructions.push_back(instruction);
}

void NoeudSeqInst::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    Noeud* p;
    for (int i = 0; i < m_instructions.size(); i++) {
        p = m_instructions[i]; // on fait pointer p sur l'element courant du vecteur
        p->traduitEnCPP(cout, 0);
    }

}


////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
    int valeur = m_expression->executer(); // On exécute (évalue) l'expression
    ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudAffectation::traduitEnCPP(ostream & cout, unsigned int indentation) const {

    m_variable->traduitEnCPP(cout, indentation);
    cout << " = ";
    m_expression->traduitEnCPP(cout, 0);
    cout << ";";
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
    int og, od, valeur;
    if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
    if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
    // Et on combine les deux opérandes en fonctions de l'opérateur
    if (this->m_operateur == "+") valeur = (og + od);
    else if (this->m_operateur == "-") valeur = (og - od);
    else if (this->m_operateur == "*") valeur = (og * od);
    else if (this->m_operateur == "==") valeur = (og == od);
    else if (this->m_operateur == "!=") valeur = (og != od);
    else if (this->m_operateur == "<") valeur = (og < od);
    else if (this->m_operateur == ">") valeur = (og > od);
    else if (this->m_operateur == "<=") valeur = (og <= od);
    else if (this->m_operateur == ">=") valeur = (og >= od);
    else if (this->m_operateur == "et") valeur = (og && od);
    else if (this->m_operateur == "ou") valeur = (og || od);
    else if (this->m_operateur == "non") valeur = (!og);
    else if (this->m_operateur == "/") {
        if (od == 0) throw DivParZeroException();
        valeur = og / od;
    }
    return valeur; // On retourne la valeur calculée
}

void NoeudOperateurBinaire::traduitEnCPP(ostream & cout, unsigned int indentation) const {

    m_operandeGauche->traduitEnCPP(cout, 0);
    cout << m_operateur.getChaine();
    m_operandeDroit->traduitEnCPP(cout, 0);

    cout << ";";
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstSi::executer() {
    if (m_condition->executer()) m_sequence->executer();
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstSi::traduitEnCPP(ostream & cout, unsigned int indentation) const {

    cout << setw(4 * indentation) << "" << "if (";
    m_condition->traduitEnCPP(cout, 0);
    cout << ") {" << endl;
    m_sequence->traduitEnCPP(cout, indentation + 1);
    cout << setw(4 * indentation) << "" << "}" << endl;

}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstTQ
////////////////////////////////////////////////////////////////////////////////

NoeudInstTQ::NoeudInstTQ(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTQ::executer() {
    while (m_condition->executer()) {
        m_sequence->executer();
    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstTQ::traduitEnCPP(ostream & cout, unsigned int indentation) const {

    cout << "while(";
    m_condition->traduitEnCPP(cout, 0);
    cout << "){" << endl;
    m_sequence->traduitEnCPP(cout, 1);
    cout << endl << "}";

}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(Noeud* affect1, Noeud* condition, Noeud* affect2, Noeud* sequence)
: m_affect1(affect1), m_condition(condition), m_incr(affect2), m_sequence(sequence) {
}

int NoeudInstPour::executer() {

    for ((m_affect1 == nullptr) ? 0 : m_affect1->executer(); m_condition->executer();) {
        m_sequence->executer();
        (m_incr == nullptr) ? : m_incr->executer();
    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstPour::traduitEnCPP(ostream & cout, unsigned int indentation) const {

    cout << "for(";
    if (m_affect1 != nullptr) {
        m_affect1->traduitEnCPP(cout, 0);
    }
    m_condition->traduitEnCPP(cout, 0);

    if (m_incr != nullptr) {
        m_incr->traduitEnCPP(cout, 0);
    }
    cout << "){" << endl;
    m_sequence->traduitEnCPP(cout, 1);
    cout << endl << "}";

}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstEcrire
////////////////////////////////////////////////////////////////////////////////

NoeudInstEcrire::NoeudInstEcrire(std::vector<Noeud*> Noeuds)
: m_vectNoeud(Noeuds) {
}

int NoeudInstEcrire::executer() {
    Noeud* p;
    for (int i = 0; i < m_vectNoeud.size(); i++) {
        p = m_vectNoeud[i]; // on fait pointer p sur l'element courant du vecteur
        if ((typeid (*p) == typeid (SymboleValue)) && (*((SymboleValue*) p) == "<CHAINE>")) {
            cout << ((SymboleValue*) p)->getChaine(); //on affiche la chaine de caractere
        } else {
            cout << p->executer(); // on affiche le résultat
        }
    }
    return 0;
}

void NoeudInstEcrire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    Noeud* p;
    for (int i = 0; i < m_vectNoeud.size(); i++) {
        p = m_vectNoeud[i]; // on fait pointer p sur l'element courant du vecteur

        p->traduitEnCPP(cout, 0);
    }


}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstLire
////////////////////////////////////////////////////////////////////////////////

NoeudInstLire::NoeudInstLire(std::vector<Noeud*> Noeuds)
: m_vectNoeud(Noeuds) {
}

int NoeudInstLire::executer() {
    Noeud* p;
    int val = 0;
    for (int i = 0; i < m_vectNoeud.size(); i++) {
        p = m_vectNoeud[i]; // on fait pointer p sur l'element courant du vecteur
        cin >> val;
        ((SymboleValue*) p)->setValeur(val);
    }

    return 0;

}

void NoeudInstLire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    Noeud* p;
    for (int i = 0; i < m_vectNoeud.size(); i++) {
        p = m_vectNoeud[i]; // on fait pointer p sur l'element courant du vecteur
        ((SymboleValue*) p)->traduitEnCPP(cout, 0);
    }
}

NoeudSiRiche::NoeudSiRiche(vector<Noeud*> & conditions, vector<Noeud*> & sequences, Noeud* sequencesinon)
: m_conditions(conditions), m_sequences(sequences), m_sequencesinon(sequencesinon) {
}

int NoeudSiRiche::executer() {
    int sortie = 0;
    int i = 0;
    while (i < m_conditions.size() && sortie == 0) {
        if (m_conditions.at(i)->executer()) {
            m_sequences.at(i)->executer();
            sortie = 1;
        }
        i++;
    }
    if (i == m_conditions.size() && sortie == 0 && m_sequencesinon != nullptr) {
        m_sequencesinon->executer();
    }

    return 0;
}

void NoeudSiRiche::traduitEnCPP(ostream & cout, unsigned int indentation) const {

    cout << setw(4 * indentation) << "" << "if (";
    m_conditions.at(0)->traduitEnCPP(cout, 0);
    cout << ") {" << endl;
    m_sequences.at(0)->traduitEnCPP(cout, indentation + 1);
    cout << setw(4 * indentation) << "" << "}" << endl;
    for (int i = 1; i < m_conditions.size() - 1; i++) {
        cout << setw(4 * indentation) << "" << "else if (";
        m_conditions.at(i)->traduitEnCPP(cout, 0);
        cout << ") {" << endl;
        m_sequences.at(i)->traduitEnCPP(cout, indentation + 1);
        cout << setw(4 * indentation) << "" << "}" << endl;
    }
    if (m_sequencesinon != nullptr) {
        cout << setw(4 * indentation) << "" << "else{" << endl;
        m_sequencesinon->traduitEnCPP(cout, indentation + 1);
        cout << setw(4 * indentation) << "" << "}" << endl;
    }

}

////////////////////////////////////////////////////////////////////////////////
// NoeudRepeter
////////////////////////////////////////////////////////////////////////////////

NoeudRepeter::NoeudRepeter(Noeud* cond, Noeud* seq)
: m_cond(cond), m_seq(seq) {
}

int NoeudRepeter::executer() {
    do {
        m_seq->executer();
    } while (m_cond->executer());
    return 0;
}

void NoeudRepeter::traduitEnCPP(ostream & cout, unsigned int indentation) const {

    cout << "do{" << endl;
    m_seq->traduitEnCPP(cout, indentation + 1);
    cout << endl << "}" << "while(";
    m_cond->traduitEnCPP(cout, 0);
    cout << ");" << endl;
}