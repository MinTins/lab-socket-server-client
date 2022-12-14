
# Взаємодія розподілених процесів через механізм сокетів

## Мета
В даній лабораторній роботі необхідно освоїти механізм (технологію)
сокетів стеку протоколів TCP/IP, зокрема його реалізацію в MS Windows.
Індивідуальний варіант роботи полягає в розробці двох програм (клієнта та
сервера, які запускаються на різних станціях мережі), розробці протоколу
обміну даними між ними та демонстрації роботи програм.

## Завдання

Гра "вгадування 4-значного числа".
Користувач на клієнті вгадує 4-значне ціле, яке зберігається на сервері. З клієнта
передаються 4 цифри, на які сервер дає відповідь із двох цифр: кількість правильних цифр та
кількість цифр на своїх місцях. Клієнт в ході гри може її завершити, почати нову гру,
завершити сеанс. Клієнт може здатися і тоді сервер розкриває загадане число. Користувач на
клієнті може вибирати спосіб задання спроб: в діалозі вводити самому чи автоматична генерація
випадкових 4-х значних чисел в заданій кількості.

## Опис протоколу
Будь-яка комунікація між сервером та клієнтом здійснюється у такому вигляді:

![*Загальний вигляд*](https://i.imgur.com/LN0W8Zy.png)

При цьому клієнт і сервер розділяють свої набори команд, для передачі даних,
або ж результату:

### Server status code:

Сервер для здійснення обробки і відповідного регулювання доступу до певних функцій використовує спеціальні статус коди, які закріплюються за кожним Socket-client. Нижче наведені відповідні пояснення загальних статус-кодів.

* 0 - Disconnected.
* 1 - Connect/Finish.
* 2 - Start/Restart.

### Клієнт (Client) (команди які подаються від клієнта):

- Команда “s” (start) розміром 1 байт – команда, за допомогою якої здійснюється початок нової гри, або ж перезапуск, і відповідно генерація нового випадкового 4-значного числа.
- Команда “fn” (finish) розміром 2 байти – команда, за допомогою якої здійснюється завершення існуючої гри достроково. У випадку якщо гра не почата, повертає сервер повертає результ-відповідь “f”.
- Команда “gup” (give up) розміром 3 байти – команда, за допомогою якої можна достроково завершити гру (здатися), і при цьому дізнатися загаданий сервером код.
- Команда “t_####”(try_number) розміром 6 байтів – команда, за допомогою якої можна спробувати при запущеній грі, вгадати яке число загадано сервером.

### Сервер (Server) *(віповіді від серверу)*:

- Команда “gn-s”(generate start) розміром 4 байти – команда, яка позначає клієнту, що сервер згенерував нову гру, а й відповідно нові випадкові числа
- Команда “gn-r”(generate restart) розміром 4 байти – команда, яка позначає клієнту, що сервер перегенерував нову гру, а й відповідно нові випадкові числа
- Команда “f”(fail) розміром 1 байт – команда, яка позначає клієнту, що сервер повернув помилку у виконанні якоїсь дії
- Команда “fn-s”(finish success) розміром 4 байти – команда, яка позначає клієнту, що сервер успішно завершив поточну гру
- Команда “gu-s_####”(give up success) розміром 9 байтів – команда, яка позначає клієнту, що сервер успішно завершив поточну гру, і при цьому повернув 4-значний код, який був загаданий. (# - [0,9])
- Команда “ws-fn”(Win success/finished) розміром 5 байтів – команда, яка позначає клієнту, що клієнт успішно завершив поточну гру, відгадавши загаданий 4-значний код.
- Команда “tf_##”(Try failed) розміром 5 байтів – команда, яка позначає клієнту, що спроба відгадати була невдала. І повертає у першій # к-ть вірних чисел, а у другій # - к-ть тих, що на своїх місцях
- Команда “tf-l”(Try failed/Limit) розміром 4 байти – команда, яка позначає клієнту, що передане число для спроби виходить за встановлені межі.
- Команда “unxp_cm”(Unexpected command) розміром 7 байтів – команда, яка позначає клієнту, що сервер отримав невідому для нього команду.
