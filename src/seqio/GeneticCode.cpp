#include "GeneticCode.h"

GeneticCode::GeneticCode()
{
  setCodeID(-1);
}

CodonMap & GeneticCode::code()
{
  return _code;
}

void GeneticCode::setCodeID(int id)
{
  _codeID = id;
}

int GeneticCode::codeID() const
{
  return _codeID;
}

char GeneticCode::operator[](string codon) const
{
  if (codon.length() != CODONLENGTH)
    throw GeneticCodeError("Codon must be three (3) characters");
    
  CodonMap::const_iterator result = _code.find(codon);
  
  if (result != _code.end())  return result->second;
  
  else  return 'X';
  
}

/*char & GeneticCode::operator[](string codon)
{
  if (codon.length() != CODONLENGTH)
    throw GeneticCodeError("Codon must be three (3) characters");
  
  return _code[codon]; 
}*/

GeneticCode GeneticCode::StandardCode()
{
  GeneticCode stdGC;
  stdGC.setCodeID(1);
  
  stdGC.code()["---"] = '-';
  
  stdGC.code()["TTT"] = 'F';
  //stdGC.code()["TTT"] = 'F';
  stdGC.code()["TTC"] = 'F';

  stdGC.code()["TTA"] = 'L';
  stdGC.code()["TTG"] = 'L';
    
  stdGC.code()["TCT"] = 'S';
  stdGC.code()["TCC"] = 'S';
  stdGC.code()["TCA"] = 'S';
  stdGC.code()["TCG"] = 'S';

  stdGC.code()["TAT"] = 'Y';
  stdGC.code()["TAC"] = 'Y';
 
  stdGC.code()["TAA"] = '*';
  stdGC.code()["TAG"] = '*';
  
  stdGC.code()["TGT"] = 'C';
  stdGC.code()["TGC"] = 'C';
 
  stdGC.code()["TGA"] = '*';
  
  stdGC.code()["TGG"] = 'W';
 
  stdGC.code()["CTT"] = 'L';
  stdGC.code()["CTC"] = 'L';
  stdGC.code()["CTA"] = 'L';
  stdGC.code()["CTG"] = 'L';
  
  stdGC.code()["CCT"] = 'P';
  stdGC.code()["CCC"] = 'P';
  stdGC.code()["CCA"] = 'P';
  stdGC.code()["CCG"] = 'P';
  
  stdGC.code()["CAT"] = 'H';
  stdGC.code()["CAC"] = 'H';

  stdGC.code()["CAA"] = 'Q';
  stdGC.code()["CAG"] = 'Q';
  
  stdGC.code()["CGT"] = 'R';
  stdGC.code()["CGC"] = 'R';
  stdGC.code()["CGA"] = 'R';
  stdGC.code()["CGG"] = 'R';
  
  stdGC.code()["ATT"] = 'I';
  stdGC.code()["ATC"] = 'I';
  stdGC.code()["ATA"] = 'I';
 
  stdGC.code()["ATG"] = 'M';
  
  stdGC.code()["ACT"] = 'T';
  stdGC.code()["ACC"] = 'T';
  stdGC.code()["ACA"] = 'T';
  stdGC.code()["ACG"] = 'T';
  
  stdGC.code()["AAT"] = 'N';
  stdGC.code()["AAC"] = 'N';

  stdGC.code()["AAA"] = 'K';
  stdGC.code()["AAG"] = 'K';
 
  stdGC.code()["AGT"] = 'S';
  stdGC.code()["AGC"] = 'S';

  stdGC.code()["AGA"] = 'R';
  stdGC.code()["AGG"] = 'R';
 
  stdGC.code()["GTT"] = 'V';
  stdGC.code()["GTC"] = 'V';
  stdGC.code()["GTA"] = 'V';
  stdGC.code()["GTG"] = 'V';

  stdGC.code()["GCT"] = 'A';
  stdGC.code()["GCC"] = 'A';
  stdGC.code()["GCA"] = 'A';
  stdGC.code()["GCG"] = 'A';

  stdGC.code()["GAT"] = 'D';
  stdGC.code()["GAC"] = 'D';

  stdGC.code()["GAA"] = 'E';
  stdGC.code()["GAG"] = 'E';
  
  stdGC.code()["GGT"] = 'G';
  stdGC.code()["GGC"] = 'G';
  stdGC.code()["GGA"] = 'G';
  stdGC.code()["GGG"] = 'G';
  
  
  /* Ambiguous codes */
  stdGC.code()["TTY"] = 'F';
  
  stdGC.code()["TTR"] = 'L';
  stdGC.code()["YTR"] = 'L';

  stdGC.code()["TCY"] = 'S';
  stdGC.code()["TCR"] = 'S';
  stdGC.code()["TCW"] = 'S';
  stdGC.code()["TCS"] = 'S';
  stdGC.code()["TCM"] = 'S';
  stdGC.code()["TCK"] = 'S';
  stdGC.code()["TCH"] = 'S';
  stdGC.code()["TCB"] = 'S';
  stdGC.code()["TCV"] = 'S';
  stdGC.code()["TCD"] = 'S';
  stdGC.code()["TCN"] = 'S';
 
  stdGC.code()["TAY"] = 'Y';
  
  stdGC.code()["TAR"] = '*';
  stdGC.code()["TRA"] = '*';

  stdGC.code()["TGY"] = 'C';

  stdGC.code()["CTY"] = 'L';  
  stdGC.code()["CTR"] = 'L';  
  stdGC.code()["CTW"] = 'L';  
  stdGC.code()["CTS"] = 'L';  
  stdGC.code()["CTM"] = 'L';  
  stdGC.code()["CTK"] = 'L';  
  stdGC.code()["CTH"] = 'L';  
  stdGC.code()["CTB"] = 'L';  
  stdGC.code()["CTV"] = 'L';  
  stdGC.code()["CTD"] = 'L';  
  stdGC.code()["CTN"] = 'L';  
  
  stdGC.code()["CCY"] = 'P';  
  stdGC.code()["CCR"] = 'P';  
  stdGC.code()["CCW"] = 'P';  
  stdGC.code()["CCS"] = 'P';  
  stdGC.code()["CCM"] = 'P';  
  stdGC.code()["CCK"] = 'P';  
  stdGC.code()["CCH"] = 'P';  
  stdGC.code()["CCB"] = 'P';  
  stdGC.code()["CCV"] = 'P';  
  stdGC.code()["CCD"] = 'P';  
  stdGC.code()["CCN"] = 'P';  
  
  stdGC.code()["CAY"] = 'H';  
  stdGC.code()["CAR"] = 'Q';  
  
  stdGC.code()["CGY"] = 'R';  
  stdGC.code()["CGR"] = 'R';  
  stdGC.code()["CGW"] = 'R';  
  stdGC.code()["CGS"] = 'R';  
  stdGC.code()["CGM"] = 'R';  
  stdGC.code()["CGK"] = 'R';  
  stdGC.code()["CGH"] = 'R';  
  stdGC.code()["CGB"] = 'R';  
  stdGC.code()["CGV"] = 'R';  
  stdGC.code()["CGD"] = 'R';  
  stdGC.code()["CGN"] = 'R';  
  
  stdGC.code()["ATY"] = 'I';
  stdGC.code()["ATM"] = 'I';
  stdGC.code()["ATW"] = 'I';
  stdGC.code()["ATH"] = 'I';
 
  stdGC.code()["ACY"] = 'T';  
  stdGC.code()["ACR"] = 'T';  
  stdGC.code()["ACW"] = 'T';  
  stdGC.code()["ACS"] = 'T';  
  stdGC.code()["ACM"] = 'T';  
  stdGC.code()["ACK"] = 'T';  
  stdGC.code()["ACH"] = 'T';  
  stdGC.code()["ACB"] = 'T';  
  stdGC.code()["ACV"] = 'T';  
  stdGC.code()["ACD"] = 'T';  
  stdGC.code()["ACN"] = 'T';  
  
  stdGC.code()["AAY"] = 'N';  

  stdGC.code()["AAR"] = 'K';  

  stdGC.code()["AGY"] = 'S';  

  stdGC.code()["AGR"] = 'R';  
  stdGC.code()["MGR"] = 'R';  
  
  stdGC.code()["GTY"] = 'V';  
  stdGC.code()["GTR"] = 'V';  
  stdGC.code()["GTW"] = 'V';  
  stdGC.code()["GTS"] = 'V';  
  stdGC.code()["GTM"] = 'V';  
  stdGC.code()["GTK"] = 'V';  
  stdGC.code()["GTH"] = 'V';  
  stdGC.code()["GTB"] = 'V';  
  stdGC.code()["GTV"] = 'V';  
  stdGC.code()["GTD"] = 'V';  
  stdGC.code()["GTN"] = 'V';  
  
  stdGC.code()["GCY"] = 'A';  
  stdGC.code()["GCR"] = 'A';  
  stdGC.code()["GCW"] = 'A';  
  stdGC.code()["GCS"] = 'A';  
  stdGC.code()["GCM"] = 'A';  
  stdGC.code()["GCK"] = 'A';  
  stdGC.code()["GCH"] = 'A';  
  stdGC.code()["GCB"] = 'A';  
  stdGC.code()["GCV"] = 'A';  
  stdGC.code()["GCD"] = 'A';  
  stdGC.code()["GCN"] = 'A';  
  
  stdGC.code()["GCN"] = 'A';  

  stdGC.code()["GCY"] = 'D';  

  stdGC.code()["GCR"] = 'E';
  
  stdGC.code()["GGY"] = 'G';  
  stdGC.code()["GGR"] = 'G';  
  stdGC.code()["GGW"] = 'G';  
  stdGC.code()["GGS"] = 'G';  
  stdGC.code()["GGM"] = 'G';  
  stdGC.code()["GGK"] = 'G';  
  stdGC.code()["GGH"] = 'G';  
  stdGC.code()["GGB"] = 'G';  
  stdGC.code()["GGV"] = 'G';  
  stdGC.code()["GGD"] = 'G';  
  stdGC.code()["GGN"] = 'G';  
  
  return stdGC;
}

GeneticCode GeneticCode::AlternateCode(int genBankID)
{
  GeneticCode altGC = StandardCode();
  altGC.setCodeID(genBankID);
  
  switch (genBankID)
  {
    case 1:
      break;

    case 2:
      altGC.code()["AGA"] = '*';  
      altGC.code()["AGG"] = '*';  
      altGC.code()["AGR"] = '*';  
 
      altGC.code()["ATA"] = 'M';  
      altGC.code()["ATR"] = 'M';
      
      altGC.code()["TGA"] = 'W';  
      altGC.code()["TGR"] = 'W';  
      break;
      
    case 3:
      altGC.code()["ATA"] = 'M';  
      altGC.code()["ATR"] = 'M';
      
      altGC.code()["CGA"] = 'X';
      altGC.code()["CGC"] = 'X';
      
      altGC.code()["CTT"] = 'T';  
      altGC.code()["CTC"] = 'T';  
      altGC.code()["CTA"] = 'T';  
      altGC.code()["CTG"] = 'T';  
      altGC.code()["CTY"] = 'T';  
      altGC.code()["CTR"] = 'T';  
      altGC.code()["CTW"] = 'T';  
      altGC.code()["CTS"] = 'T';  
      altGC.code()["CTM"] = 'T';  
      altGC.code()["CTK"] = 'T';  
      altGC.code()["CTH"] = 'T';  
      altGC.code()["CTB"] = 'T';
      altGC.code()["CTV"] = 'T';  
      altGC.code()["CTD"] = 'T';
      altGC.code()["CTN"] = 'T';

      altGC.code()["TGA"] = 'W';  
      altGC.code()["TGR"] = 'W';  
      break;
      
    case 4:
      altGC.code()["TGA"] = 'W';  
      altGC.code()["TGR"] = 'W';  
      break;
      
    case 5:
      altGC.code()["AGA"] = 'S';  
      altGC.code()["AGG"] = 'S';  
      altGC.code()["AGY"] = 'S';  
      altGC.code()["AGR"] = 'S';  
      altGC.code()["AGW"] = 'S';  
      altGC.code()["AGS"] = 'S';  
      altGC.code()["AGM"] = 'S';  
      altGC.code()["AGK"] = 'S';  
      altGC.code()["AGH"] = 'S';  
      altGC.code()["AGB"] = 'S';  
      altGC.code()["AGV"] = 'S';  
      altGC.code()["AGD"] = 'S';  
      altGC.code()["AGN"] = 'S';  
     
      altGC.code()["ATA"] = 'M';  
      altGC.code()["ATR"] = 'M';

      altGC.code()["TGA"] = 'W';  
      altGC.code()["TGR"] = 'W';  
      break;
      
    case 6:
      altGC.code()["TAA"] = 'Q';  
      altGC.code()["TAG"] = 'Q';  
      altGC.code()["TAR"] = 'Q';  
      altGC.code()["YAA"] = 'Q';  
      altGC.code()["YAG"] = 'Q';  
      altGC.code()["YAR"] = 'Q';  
      break;
      
    case 9:
      altGC.code()["AAA"] = 'N';  
      altGC.code()["AAM"] = 'N';  
      altGC.code()["AAW"] = 'N';  
      altGC.code()["AAH"] = 'N';  

      altGC.code()["AGA"] = 'S';  
      altGC.code()["AGG"] = 'S';  
      altGC.code()["AGY"] = 'S';  
      altGC.code()["AGR"] = 'S';  
      altGC.code()["AGW"] = 'S';  
      altGC.code()["AGS"] = 'S';  
      altGC.code()["AGM"] = 'S';  
      altGC.code()["AGK"] = 'S';  
      altGC.code()["AGH"] = 'S';  
      altGC.code()["AGB"] = 'S';  
      altGC.code()["AGV"] = 'S';  
      altGC.code()["AGD"] = 'S';  
      altGC.code()["AGN"] = 'S';  

      altGC.code()["TGA"] = 'W';  
      altGC.code()["TGR"] = 'W';  
      break;
      
    case 10:
      altGC.code()["TGA"] = 'C';  
      altGC.code()["TGW"] = 'C';  
      altGC.code()["TGM"] = 'C';  
      altGC.code()["TGH"] = 'C';  
      break;

    case 11:
      break;
      
    case 12:
      altGC.code()["CTG"] = 'S';  
      break;
      
    case 13:
      altGC.code()["AGA"] = 'G';  
      altGC.code()["AGG"] = 'G';  
      altGC.code()["AGR"] = 'G';  
      altGC.code()["RGA"] = 'G';  
      altGC.code()["RGG"] = 'G';  
      altGC.code()["RGR"] = 'G';  
     
      altGC.code()["ATA"] = 'M';  
      altGC.code()["ATR"] = 'M';

      altGC.code()["TGA"] = 'W';  
      altGC.code()["TGR"] = 'W';  
      break;
      
    case 14:
      altGC.code()["AAA"] = 'N';  
      altGC.code()["AAM"] = 'N';  
      altGC.code()["AAW"] = 'N';  
      altGC.code()["AAH"] = 'N';  

      altGC.code()["AGA"] = 'S';  
      altGC.code()["AGG"] = 'S';  
      altGC.code()["AGY"] = 'S';  
      altGC.code()["AGR"] = 'S';  
      altGC.code()["AGW"] = 'S';  
      altGC.code()["AGS"] = 'S';  
      altGC.code()["AGM"] = 'S';  
      altGC.code()["AGK"] = 'S';  
      altGC.code()["AGH"] = 'S';  
      altGC.code()["AGB"] = 'S';  
      altGC.code()["AGV"] = 'S';  
      altGC.code()["AGD"] = 'S';  
      altGC.code()["AGN"] = 'S';  

      altGC.code()["TAA"] = 'Y';  
      altGC.code()["TAM"] = 'Y';  
      altGC.code()["TAW"] = 'Y';  
      altGC.code()["TAH"] = 'Y'; 
      
      altGC.code()["TGA"] = 'W';  
      altGC.code()["TGR"] = 'W';
      break;

    case 15:
      altGC.code()["TAG"] = 'Q';  
      altGC.code()["YAG"] = 'Q';
      break;
      
    case 16:
      altGC.code()["TAG"] = 'L';
      altGC.code()["TWG"] = 'L';
      break;

    case 21:
      altGC.code()["AGA"] = 'S';  
      altGC.code()["AGG"] = 'S';  
      altGC.code()["AGY"] = 'S';  
      altGC.code()["AGR"] = 'S';  
      altGC.code()["AGW"] = 'S';  
      altGC.code()["AGS"] = 'S';  
      altGC.code()["AGM"] = 'S';  
      altGC.code()["AGK"] = 'S';  
      altGC.code()["AGH"] = 'S';  
      altGC.code()["AGB"] = 'S';  
      altGC.code()["AGV"] = 'S';  
      altGC.code()["AGD"] = 'S';  
      altGC.code()["AGN"] = 'S';  
     
      altGC.code()["ATA"] = 'M';  
      altGC.code()["ATR"] = 'M';

      altGC.code()["TGA"] = 'W';  
      altGC.code()["TGR"] = 'W';  

      altGC.code()["AAA"] = 'N';
      altGC.code()["AAH"] = 'N';
      break;

    case 22:
      altGC.code()["TCA"] = '*';
      altGC.code()["TMA"] = '*';
      altGC.code()["TSA"] = '*';
      altGC.code()["TVA"] = '*';
      
      altGC.code()["TAG"] = 'L';
      altGC.code()["TWG"] = 'L';
      break;

    case 23:
      break;
      
    default:
      throw GeneticCodeError("Unknown genetic code");
      break;
  }
  
  return altGC;
}

string GeneticCode::lookupCode(int genBankID)
{
  switch(genBankID)
  {
    case 1:
      return "Universal Code";
      break;
    case 2:
      return "Vertebrate Mitochondrial Code";
      break;
    case 3:
      return "Yeast Mitochondrial Code";
      break;
    case 4:
      return "Mould Mitochondrial Code";
      break;
    case 5:
      return "Invertebrate Mitochondrial Code";
      break;
    case 6:
      return "Ciliate Code";
      break;
    case 9:
      return "Flatworm/Echinoderm Mitochondrial Code";
      break;
    case 10:
      return "Euplotid Code";
      break;
    case 11:
      return "Bacterial and Plastid Code";
      break;
    case 12:
      return "Alternate Yeast Code";
      break;
    case 13:
      return "Ascidian Mitochondrial Code";
      break;
    case 14:
      return "Alternate Flatworm Mitochondrial Code";
      break;
    case 15:
      return "Blepharisma Code";
      break;
    case 16:
      return "Green Algae Mitochondrial Code";
      break;
    case 21:
      return "Trematode Mitochondrial Code";
      break;
    case 22:
      return "Scenedesmus obliquus Mitochondrial Code";
      break;
    case 23:
      return "Thraustochytrium aureum Mitochondrial Code";
      break;
    default:
      return "Unknown Genetic Code";
      break;
  }
}

ostream &operator<<(ostream &out, GeneticCode &gc)
{
  string nucleotides = "TCAG";
  int nnucs = nucleotides.length();
  char codon[4];
  codon[3] = '\0';
  
  out << gc.lookupCode(gc.codeID()) << ":\n\n";
  for (int i = 0; i < nnucs; i++)
  {
    codon[0] = nucleotides.at(i);
    for (int j = 0; j < nnucs; j++)
    {
      codon[2] = nucleotides.at(j);
      for (int k = 0; k < nnucs; k++)
      {
        codon[1] = nucleotides.at(k);
        out << codon << " " << gc[codon] << "  ";
      }
      out << "\n";
    }
    out << endl;
  }
  
  return out;
}

GeneticCodeError::GeneticCodeError()
  : exception()
{
  string s = "Unspecified error in GeneticCode class.";
  _msg = new char[s.length() + 1];
  s.copy(_msg, s.length());
  _msg[s.length()] = '\0';
}
  
GeneticCodeError::GeneticCodeError(const char *const& message)
  : exception()
{
  size_t length = strlen(message);
  _msg = new char[length + 1];
  strcpy(_msg, message);
  _msg[length] = '\0';
  
}

GeneticCodeError::~GeneticCodeError() throw()
{
  delete _msg;
}

const char* GeneticCodeError::what() const throw()
{
  return _msg;
}

