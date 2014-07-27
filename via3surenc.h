
int hdSurEncBasicCrypt_D2_0F_11(int Value, int XorVal);
int hdSurEncCryptLookup_D2_0F_11(unsigned char Value, unsigned char AddrInd);		
void hdSurEncPhase1_D2_0F_11(unsigned char *CWs);
void hdSurEncPhase2_D2_0F_11_sub(unsigned char *CWa, unsigned char *CWb, unsigned char AddrInd);		
void hdSurEncPhase2_D2_0F_11(unsigned char *CWs);		
void CommonMain_1_D2_13_15(const unsigned char *datain, unsigned char *dataout);		
unsigned short CommonMain_2_D2_13_15(const unsigned char *data, const unsigned long num);		
void CommonMain_3_D2_13_15(unsigned char *data0, unsigned char *data1, int nbrloop);		
void CommonMain_D2_13_15(const unsigned char *datain, unsigned char *dataout, int loopval);		
void Common_D2_13_15(unsigned char *cw0, const unsigned char *cw1, int loopval);		
void ExchangeCWs(unsigned char *cw0, unsigned char *cw1);
void hdSurEncPhase1_D2_13_15(unsigned char *cws);
void hdSurEncPhase2_D2_13_15(unsigned char *cws);
