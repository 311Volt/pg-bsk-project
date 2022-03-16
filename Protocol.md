
## KEX:

```C++
enum MSG_TYPE {
    KEX_INIT,
    KEX_RESPONSE
    MSG,
    FILE_META,
    FILE_BLOCK
};
enum ERR_CODE {
    SUCCESS,
    FAILED,
    FAILED_VALIDATION
};
enum CIPHER_VARIANT {
    CHACHA20_POLY1305,
    CHACHA20
};
struct KexMessage {
    ERR_CODE error_code;
    MSG_TYPE msg_type;  // == KEX_INIT | KEX_RESPONSE
    string username;
    uint8_t ecdhe_pubkey[33];
    uint8_t user_pubkey[33];
    string ipaddress;
    int port;
    uint8_t message_signature[64];
};
struct Message {
    MSG_TYPE msg_type;  // == MSG
    CIPHER_VARIANT cipher_variant;
    uint8_t nonce[12];
    std::vector<uint8_t> encrypted_data;
    uint8_t mac[16]; // optional when using CHACHA20_POLY1305
};
struct MessageFileMeta {
    MSG_TYPE msg_type;  // == FILE_META
    CIPHER_VARIANT cipher_variant;
    uint8_t nonce[12];
    std::vector<uint8_t> encrypted_meta;    // file name + file length + file sha512
    uint8_t mac[16]; // optional when using CHACHA20_POLY1305
};
struct MessageFileBlock {
    MSG_TYPE msg_type;  // == FILE_BLOCK
    CIPHER_VARIANT cipher_variant;
    uint8_t nonce[12];
    std::vector<uint8_t> encrypted_block;    // file hash + offset bytes + block
    uint8_t mac[16]; // optional when using CHACHA20_POLY1305
};
```

Inicjalizacja połączenia:

`A` wysyła `KexMessage Kex(KexMessage);` do `B` i czeka na odpowiedź, po czym połączenie jest ustanowione (dla uproszczenia, robimy jedno połącznie pomiędzy dwoma klientami, żeby nie było problemów z RPC i adresami ip).

Po tym dowolna strona wysyła dowolną wiadomość:
```C++
ERR_CODE SendMessage(Message);
ERR_CODE SendFileMeta(Message);
ERR_CODE SendFileBlock(Message);
```

Jeszcze można dla 'bezpieczeństwa' dodać po odebraniu wiadomości `KexMessage` zaptanie RPC, czy połączyłem się z tym samym który wysyłał prośbę o połączenie. Kex jest przy okazji prośbą o połączenie.
