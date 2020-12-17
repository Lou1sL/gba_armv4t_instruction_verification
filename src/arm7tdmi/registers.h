
union StatusRegister {
    struct { unsigned int m:5, t:1, f:1, i:1, r:19, q:1, v:1, c:1, z:1, n:1; };
    struct { unsigned int qriftm:28, nzcv:4;                                 };
    std::uint32_t value;
    template<typename T>
    StatusRegister &operator=(const T& other) {
        if constexpr (std::is_same_v<T, StatusRegister>) { value = other.value; }
        else { value = other; }
        return *this;
    }
};

struct RegisterGroup {
    std::uint32_t current[GENERAL_REG_SIZE];
    std::uint32_t &operator[](std::size_t i){ return current[i]; }
    std::uint32_t banked[BANK_COUNT][BANK_SIZE];
    StatusRegister cpsr, spsr[BANK_COUNT];
    StatusRegister* p_spsr;
    void Reset(){
        for(std::size_t i=0;i<GENERAL_REG_SIZE;i++) current[i] = 0;
        for(std::size_t i=0;i<BANK_COUNT;i++){
            for(std::size_t j=0;j<BANK_SIZE;j++) banked[i][j] = 0;
            spsr[i] = 0;
        }
        cpsr = 0;
    }
    BANK GetBankByMode(MODE m){
        switch (m) {
            case MODE_USR: case MODE_SYS: return BANK_SYSUSR;
            case MODE_FIQ: return BANK_FIQ;
            case MODE_IRQ: return BANK_IRQ;
            case MODE_SVC: return BANK_SVC;
            case MODE_ABT: return BANK_ABT;
            case MODE_UND: return BANK_UND;
        }
        return BANK_UND;
    }
    void SwitchToMode(MODE m){
        BANK o = GetBankByMode((MODE)cpsr.m), n = GetBankByMode(m);
        cpsr.m = m;
        p_spsr = n == BANK_SYSUSR ? &cpsr : &spsr[n];
        if(o == n) return;
        for (std::size_t i = 0; i < BANK_SIZE; i++) {
            if((o != BANK_FIQ && n != BANK_FIQ) && (i != 5 && i != 6)) continue;
            banked[o][i] = current[8+i];
            current[8+i] = banked[n][i];
        }
    }
};