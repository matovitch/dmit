//-----------------------------------------------------------------------------
// Based upon the implementation of SHA256 by Brad Conte (brad AT bradconte.com):
// https://github.com/B-Con/crypto-algorithms/blob/master/sha256.c
//
#include "dmit/com/sha256.hpp"

#include "dmit/com/unique_id.hpp"

#include <cstdint>
#include <cstring>

#define DMIT_COM_SHA256_ROTLEFT(a,b)  (((a) << (b)) | ((a) >> (32 - (b))))
#define DMIT_COM_SHA256_ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32 - (b))))

#define DMIT_COM_SHA256_CH(x,y,z)  (((x) & (y)) ^ (~(x) & (z)))
#define DMIT_COM_SHA256_MAJ(x,y,z) (((x) & (y)) ^ ( (x) & (z)) ^ ((y) & (z)))

#define DMIT_COM_SHA256_EP0(x) (DMIT_COM_SHA256_ROTRIGHT(x,  2) ^ \
                                DMIT_COM_SHA256_ROTRIGHT(x, 13) ^ \
                                DMIT_COM_SHA256_ROTRIGHT(x, 22))
#define DMIT_COM_SHA256_EP1(x) (DMIT_COM_SHA256_ROTRIGHT(x,  6) ^ \
                                DMIT_COM_SHA256_ROTRIGHT(x, 11) ^ \
                                DMIT_COM_SHA256_ROTRIGHT(x, 25))
#define DMIT_COM_SHA256_SIG0(x) (DMIT_COM_SHA256_ROTRIGHT(x,  7) ^ \
                                 DMIT_COM_SHA256_ROTRIGHT(x, 18) ^ ((x) >> 3))
#define DMIT_COM_SHA256_SIG1(x) (DMIT_COM_SHA256_ROTRIGHT(x, 17) ^ \
                                 DMIT_COM_SHA256_ROTRIGHT(x, 19) ^ ((x) >> 10))

static const uint32_t k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

namespace dmit::com::sha256
{

void transform(const uint8_t* const bytes, State& state)
{
    uint32_t a, b, c, d, e, f, g, h, t1, t2, m[64];

    for (int i = 0, j = 0; i < 16; ++i, j += 4)
    {
        m[i] = (bytes[j] << 24) | (bytes[j + 1] << 16) | (bytes[j + 2] << 8) | (bytes[j + 3]);
    }

    for (int i = 16; i < 64; ++i)
    {
        m[i] = DMIT_COM_SHA256_SIG1(m[i -  2]) + m[i -  7] + 
               DMIT_COM_SHA256_SIG0(m[i - 15]) + m[i - 16];
    }

    a = state._asUint32[0];
    b = state._asUint32[1];
    c = state._asUint32[2];
    d = state._asUint32[3];
    e = state._asUint32[4];
    f = state._asUint32[5];
    g = state._asUint32[6];
    h = state._asUint32[7];

    for (int i = 0; i < 64; ++i)
    {
        t1 = h + DMIT_COM_SHA256_EP1(e) + DMIT_COM_SHA256_CH(e,f,g) + k[i] + m[i];
        t2 = DMIT_COM_SHA256_EP0(a) + DMIT_COM_SHA256_MAJ(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    state._asUint32[0] += a;
    state._asUint32[1] += b;
    state._asUint32[2] += c;
    state._asUint32[3] += d;
    state._asUint32[4] += e;
    state._asUint32[5] += f;
    state._asUint32[6] += g;
    state._asUint32[7] += h;
}

void update(const uint8_t* const bytes, const uint16_t size, State& state)
{
    static uint8_t buffer[64];

    for (int i = 0; i < size >> 6; ++i) {
        std::memset(buffer, 0                , sizeof(buffer));
        std::memcpy(buffer, bytes + (i << 6) , sizeof(buffer));
        transform(buffer, state);
    }
}

const UniqueId& State::uniqueId() const
{
    return reinterpret_cast<const UniqueId&>(*this);
}

UniqueIdSequence::UniqueIdSequence(const UniqueId& seed)
{
    static uint8_t buffer[64];

    for (int i = 0; i < sizeof(seed); i++)
    {
        buffer[i << 0] = reinterpret_cast<const uint8_t*>(&seed)[i];
        buffer[i << 1] = reinterpret_cast<const uint8_t*>(&seed)[i];
    }

    for (int i = 0; i < sizeof(_state); i++)
    {
        buffer[i + 32] = reinterpret_cast<const uint8_t*>(&_state)[i];
    }

    update(buffer, sizeof(buffer), _state);
}

const UniqueId& UniqueIdSequence::operator()()
{
    static uint8_t buffer[64];

    for (int i = 0; i < sizeof(_state); i++)
    {
        buffer[i << 0] = reinterpret_cast<const uint8_t*>(&_state)[i];
        buffer[i << 1] = reinterpret_cast<const uint8_t*>(&_state)[i];
    }

    update(buffer, sizeof(buffer), _state);

    return _state.uniqueId();
}

} // namespace dmit::com::sha256
