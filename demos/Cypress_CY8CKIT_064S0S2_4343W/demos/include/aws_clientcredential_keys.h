#ifndef AWS_CLIENT_CREDENTIAL_KEYS_H
#define AWS_CLIENT_CREDENTIAL_KEYS_H

#include <stdint.h>

/*
 * PEM-encoded client certificate.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----"
 */
#define keyCLIENT_CERTIFICATE_PEM \
"-----BEGIN CERTIFICATE-----\n"\
"MIIDWTCCAkGgAwIBAgIUWiTn5GA6HWpc1SxDwgDjK5T3BmwwDQYJKoZIhvcNAQEL\n"\
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"\
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIwMDkwMjEzNDQ1\n"\
"MVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"\
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOQKM2YNpYSBwWcq68bK\n"\
"9MsSXn/nFWH6AD2Bppy9q6/b+fuQL7bvYbqtsiQ0Pkh1Tjz8mUVlcAD2wdmFxf6U\n"\
"yIQhxVXBs0cHo9N7M6A4VwPSTDyV4LLA2UXQLpKQVCBdGBMhC15Y6l5QMvNxZQKg\n"\
"jzwA0KyTiQq5H942vmUg/BEqo9gZTNWjImUWseF/+bUxtjKGo8+im8ATJyLC2e9G\n"\
"e4hjx8M6Ny3gxdYTLtHgqHGZo0KCIfNB5PU4O/yNZ3ZXgw93GVag/NjwYxat2wvk\n"\
"7AuwbH+e5gJNbJnMxB5aiwrKO/9tjfMe8MUfkNZiXIcLQECZSuYHMUTHNhl7b6kA\n"\
"pHcCAwEAAaNgMF4wHwYDVR0jBBgwFoAU/0dNQ3WwFYiTaKPk0CK/+PASKB0wHQYD\n"\
"VR0OBBYEFIrdZkUt3lfpLewPpcywA9O59RX+MAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"\
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCLQ8Ltu3UNMtSOl4nwjX0tc/Mt\n"\
"JJ7VJU/dEHo6vXih5JUAZ8RUafF8tXAytOFTFuN19XEegHMDdi8VGlWeslHtcCSG\n"\
"KtGz7bnaHDnK9C8nhqttIJiPl81OU/1plHMx2D3+/jC1gEcUSD1/v6og9Sj7/boL\n"\
"3rmium2ZSdEZFo+Bb/nq+kYRMbcJ5eukh1Qy1/83vEgEQl1xVH5owoNnFJpsDgVb\n"\
"1xbJIxk/x1vnWSDx49wsrULSb8ToCOuu0DuMppkP3Bbl0JGxYjlks122GM1b8UdV\n"\
"M8IBeKgh2TCojQY4d0tM5uo5rN3H9ybMcqt5GSvzdmEhzkr1o+GitBp3tgg+\n"\
"-----END CERTIFICATE-----"

/*
 * PEM-encoded client private key.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN RSA PRIVATE KEY-----\n"\
 * "...base64 data...\n"\
 * "-----END RSA PRIVATE KEY-----"
 */
#define keyCLIENT_PRIVATE_KEY_PEM \
"-----BEGIN RSA PRIVATE KEY-----\n"\
"MIIEowIBAAKCAQEA5AozZg2lhIHBZyrrxsr0yxJef+cVYfoAPYGmnL2rr9v5+5Av\n"\
"tu9huq2yJDQ+SHVOPPyZRWVwAPbB2YXF/pTIhCHFVcGzRwej03szoDhXA9JMPJXg\n"\
"ssDZRdAukpBUIF0YEyELXljqXlAy83FlAqCPPADQrJOJCrkf3ja+ZSD8ESqj2BlM\n"\
"1aMiZRax4X/5tTG2Moajz6KbwBMnIsLZ70Z7iGPHwzo3LeDF1hMu0eCocZmjQoIh\n"\
"80Hk9Tg7/I1ndleDD3cZVqD82PBjFq3bC+TsC7Bsf57mAk1smczEHlqLCso7/22N\n"\
"8x7wxR+Q1mJchwtAQJlK5gcxRMc2GXtvqQCkdwIDAQABAoIBADbdnXA4aFJxRa8k\n"\
"zIWP30XcrY/ocWSZWeVyhDvCBsTREFxOXKyO+9yGXg4H49RbJO/XdqkTttcEh69m\n"\
"WDTgZz4jQe9YI2I7nFcNlWY/J/BrcJxDE0Tu0VfStuHch+EhhOFVWf7hNcJrWVbp\n"\
"OXxgVRYCvDJJbPd/gwNb+IMsZHlcbLsSBAm0YlGqUsRIZ4/VI2fSrNiLot301mWl\n"\
"FrGwnh+ClCOXsMRvmYcNVdIC8+WKCMGOC6EyJrWw4BWkUd9UrPP1vfNVIliDylli\n"\
"vI2D/Z2RJGXV/WvIcme1uFN0hTnb5cACmZP8gfjLz9pknvYpA6z0AHgKS742BDDv\n"\
"8pwzZQECgYEA8pI55lK22o5NH1jruTJdUxAadcbyzdgS3n2usLc7sqjGupZ5jI9/\n"\
"TtUnOL4oQ7fPSbDgQ2FTGJFttHCuVr6rGieReYy+r9ZVUTRjkbOmnimffhTxdv8X\n"\
"DoXDCaNNZmj0UhNVc4l5veYkqLp1rwNzEPYQ195LM8hsOA2Qc5OcCNECgYEA8KoI\n"\
"dUH5dqDwuaapGc1ObMuP0mn2JUq3m7rXseBAjzQGt5yFG/CXRQU58SbuMvjygvGB\n"\
"cb160GszguUHmMaiNZd0mDzpF6qTOvkRK9tqrGmvOie/sb5oqWh1ym/uvUV4Z3S5\n"\
"yDgw2z5ZtgDWDlyhPyt/AnxTHOjHiSsnECXrqscCgYEA6riEPlMbjTDVf9fTgUky\n"\
"9QVIFV+F45mJ6LFFMMqdgau/YR3qUBEq52VqpoWbAKwEcRIucATlG4jd1xSSHm4x\n"\
"swVGx49hlVSZChLfpkVonPM52g75+GOuM+dLazAR3V8By8nZjgshOphQMNT8u3Vj\n"\
"Cq6QbdI0gs8VqMe3V58mBYECgYBQXLbOUOV4U22O0LQOxZu2gJLZ9EAZW8XL3qw/\n"\
"3V4xc3H6xe080y3wAjrC5/kKVE7GotZSoK7uAZ4Fy2yp+0oXEyf/3fcHzBG1042E\n"\
"U5RfVjsI9FTyiV5xk0wh+RaDZTs7cKLI1Nqbm0phmA3iZdQCQAf9UH6AXMqkZyB4\n"\
"J/F16QKBgFtFRqXfGhtZnTBkBd0F/UY5Mv3gTN7AV7SiLJKdHs400GHzSrtQ2qWf\n"\
"IzZ6gj7fRzcwx5pSCuxTwlR8KUwJZWxGr7+qXC2xw1p4eL2LOnTVw33J7HoCoXWR\n"\
"UJVFRpgtTSRO7DvMhUVQeqR6FU/jHH2wxc9+WDEycapjPlXUflgG\n"\
"-----END RSA PRIVATE KEY-----"

/*
 * PEM-encoded Just-in-Time Registration (JITR) certificate (optional).
 *
 * If used, must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----"
 */
#define keyJITR_DEVICE_CERTIFICATE_AUTHORITY_PEM  ""


#endif /* AWS_CLIENT_CREDENTIAL_KEYS_H */
