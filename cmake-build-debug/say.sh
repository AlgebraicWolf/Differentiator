curl -X POST -H "Authorization: Bearer CggVAgAAABoBMxKABA6JPy8d-zgKzqxHlwZ588MLg4v-O3m5R20e5b16FsLkI_Z_DhaHc3EWlsNLVQWPubEegn2wn1oWbxggqu6d-EledJelZ4_NDRu5aULiJqiClQ68ICbZOx5qnVrM0_7TRZJz3yTdUpK-inXHEuqtZOE8TmX0bSevGCcQ_wMVCZ5ryY-8llbi61_IlJGEVL6hU4TKAj5uPZuDP1n1vpXxmxpRRsralkBiKwF27sic1Qh9349w-0unyZ244vFNKiTpuYf8BlrjswrUyXJFLILmbm68PW60FAYaoRzIWSORbFyFqVIlXINhONKcnhbho1nrVq-kUc5_VzJECmxZ_uF36xQ3EJoQssLIg_JpK1lUayets_oPPeXIotcxJa55klfmVbyVSDDOGVm5zqs_qDRiax0xLzug1cRG8t0ZTw3pltZoyFfYWA7Bw5DDAVzIHD0cpjKK7ZwxvqGguMpzUhccwJaAKu3i9WMjFYCnpI9gor2ix6YtUp_PnLtTgDNP8trO0PFKG-M87IkAAztTrIwUG3W7EUfVZ40Kkq8eIiUDpWfkj2BtvZJyDEBE7yCO-z5Q0vJwrmge-S3ryanIJE0dt-lkOpfhet4nPsjEBvwPM7x2Fd4ZdsxiNaAZU8SkzaX1zPowoKLhNLhswDlrjIPSVj464cq3ePbIvt7eQc4yHnkXGiQQy5yp7wUYi-6r7wUiFgoUYWplOG9xNGxlb3U3bmtpaG5iNzQ=" --data-urlencode "text=$1" -d "lang=ru-RU&folderId=b1gkl91stjqm4c6fvve8&format=lpcm&sampleRateHertz=48000&emotion=$3&voice=$2" "https://tts.api.cloud.yandex.net/speech/v1/tts:synthesize" | sox -r 48000 -b 16 -e signed-integer -c 1 -t raw - -t wav - | paplay