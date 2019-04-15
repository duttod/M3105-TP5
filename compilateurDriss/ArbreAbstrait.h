#ifndef ARBREABSTRAIT_H
#define ARBREABSTRAIT_H

// Contient toutes les déclarations de classes nécessaires
//  pour représenter l'arbre abstrait

#include <vector>
#include <iostream>
#include <iomanip>
using namespace std;

#include "Symbole.h"
#include "Exceptions.h"

////////////////////////////////////////////////////////////////////////////////
class Noeud {
// Classe abstraite dont dériveront toutes les classes servant à représenter l'arbre abstrait
// Remarque : la classe ne contient aucun constructeur
  public:
    virtual int  executer() =0 ; // Méthode pure (non implémentée) qui rend la classe abstraite
    virtual void traduitEnCPP(ostream & cout,unsigned int indentation) const =0;
    virtual void ajoute(Noeud* instruction) { throw OperationInterditeException(); }
    virtual ~Noeud() {} // Présence d'un destructeur virtuel conseillée dans les classes abstraites
};

////////////////////////////////////////////////////////////////////////////////
class NoeudSeqInst : public Noeud {
// Classe pour représenter un noeud "sequence d'instruction"
//  qui a autant de fils que d'instructions dans la séquence
  public:
     NoeudSeqInst();   // Construit une séquence d'instruction vide
    ~NoeudSeqInst() {} // A cause du destructeur virtuel de la classe Noeud
    void traduitEnCPP(ostream & cout,unsigned int indentation) const;
    int executer();    // Exécute chaque instruction de la séquence
    void ajoute(Noeud* instruction);  // Ajoute une instruction à la séquence

  private:
    vector<Noeud *> m_instructions; // pour stocker les instructions de la séquence
};

////////////////////////////////////////////////////////////////////////////////
class NoeudAffectation : public Noeud {
// Classe pour représenter un noeud "affectation"
//  composé de 2 fils : la variable et l'expression qu'on lui affecte
  public:
     NoeudAffectation(Noeud* variable, Noeud* expression); // construit une affectation
     void traduitEnCPP(ostream & cout,unsigned int indentation) const ;
    ~NoeudAffectation() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();        // Exécute (évalue) l'expression et affecte sa valeur à la variable

  private:
    Noeud* m_variable;
    Noeud* m_expression;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudOperateurBinaire : public Noeud {
// Classe pour représenter un noeud "opération binaire" composé d'un opérateur
//  et de 2 fils : l'opérande gauche et l'opérande droit
  public:
    NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit);
    // Construit une opération binaire : operandeGauche operateur OperandeDroit
   ~NoeudOperateurBinaire() {} // A cause du destructeur virtuel de la classe Noeud
    void traduitEnCPP(ostream & cout,unsigned int indentation) const ;
    int executer();            // Exécute (évalue) l'opération binaire)

  private:
    Symbole m_operateur;
    Noeud*  m_operandeGauche;
    Noeud*  m_operandeDroit;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudInstSi : public Noeud {
// Classe pour représenter un noeud "instruction si"
//  et ses 2 fils : la condition du si et la séquence d'instruction associée
  public:
    NoeudInstSi(Noeud* condition, Noeud* sequence);
     // Construit une "instruction si" avec sa condition et sa séquence d'instruction
   ~NoeudInstSi() {} // A cause du destructeur virtuel de la classe Noeud
    void traduitEnCPP(ostream & cout,unsigned int indentation) const ;
    int executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence

  private:
    Noeud*  m_condition;
    Noeud*  m_sequence;
};

class NoeudInstTQ : public Noeud {
// Classe pour représenter un noeud "tant que"
//  et ses 2 fils : la condition du tant que et la séquence d'instruction associée
  public:
    NoeudInstTQ(Noeud* condition, Noeud* sequence);
     // Construit une "instruction tant que" avec sa condition et sa séquence d'instruction
   ~NoeudInstTQ() {} // A cause du destructeur virtuel de la classe Noeud
    void traduitEnCPP(ostream & cout,unsigned int indentation) const ;
    int executer();  // Exécute l'instruction tant que condition vraie on exécute la séquence

  private:
    Noeud*  m_condition;
    Noeud*  m_sequence;
};

class NoeudInstPour : public Noeud {
// Classe pour représenter un noeud "pour"
  public:
    NoeudInstPour(Noeud* affect1 ,Noeud* condition,Noeud* affect2, Noeud* sequence);
     // Construit une "instruction pour" 
   ~NoeudInstPour() {} // A cause du destructeur virtuel de la classe Noeud
    void traduitEnCPP(ostream & cout,unsigned int indentation) const ;
    int executer();  // Exécute l'instruction tant que condition vraie on exécute la séquence + les 2 affectations

  private:
     
    Noeud*  m_affect1;
    Noeud*  m_condition;
    Noeud*  m_incr;
    Noeud*  m_sequence;

};

class NoeudInstEcrire : public Noeud {

  public:
    NoeudInstEcrire( std::vector<Noeud*> Noeuds );
     // Construit une "instruction tant que" avec sa condition et sa séquence d'instruction
   ~NoeudInstEcrire() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();  // Exécute l'instruction tant que condition vraie on exécute la séquence
    void traduitEnCPP(ostream & cout,unsigned int indentation) const ;

  private:
     
    std::vector<Noeud*> m_vectNoeud;
};

class NoeudInstLire : public Noeud {

  public:
    NoeudInstLire( std::vector<Noeud*> Noeuds );
     // Construit une "instruction lire" avec sa suite de variable
   ~NoeudInstLire() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();  // Exécute l'instruction tant que condition vraie on exécute la séquence
    void traduitEnCPP(ostream & cout,unsigned int indentation) const ;

  private:
     
    std::vector<Noeud*> m_vectNoeud;
};

#endif /* ARBREABSTRAIT_H */
