
class BusInterface {
public:
    virtual std::uint8_t  R8   (std::uint32_t addr, bool isSeq) = 0;
    virtual std::uint16_t R16  (std::uint32_t addr, bool isSeq) = 0;
    virtual std::uint32_t R32  (std::uint32_t addr, bool isSeq) = 0;
    virtual void          W8   (std::uint32_t addr, std::uint8_t  val, bool isSeq) = 0;
    virtual void          W16  (std::uint32_t addr, std::uint16_t val, bool isSeq) = 0;
    virtual void          W32  (std::uint32_t addr, std::uint32_t val, bool isSeq) = 0;
    virtual void          ICyc (){};
};