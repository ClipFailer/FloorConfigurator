# FloorConfigurator — Тестовое задание PlayEstate

Интерактивный конфигуратор этажей и квартир с парсингом JSON,
навигацией камеры по трём режимам и UMG-интерфейсом.

## Управление

| Действие | Ввод |
|---|---|
| Выбор квартиры | ЛКМ по 3D-объекту |
| Орбита в Genplan | ПКМ (зажать) + движение мыши |
| Выбор этажа | Кнопки 1..N в верхней панели |
| Возврат в Genplan | Кнопка «Общий вид» |
| Откат действия | Кнопка «Назад» |
| Скрыть проданные | Чекбокс в верхней панели |

## Архитектура

### Данные
- **FConfigLoader** — читает `Config/config.json`, валидирует квартиры
  (ID > 0, Area > 0, Status не пустой), удаляет невалидные записи
  и пустые этажи. Не падает на битом JSON.
- **UFloorConfiguratorSubsystem** (GameInstanceSubsystem) — хранит конфиг,
  состояние навигации (`FViewState`) и историю (`StateHistory`) для «Назад».
  Кеширует FocusPoint'ы здания/этажей и размер квартиры в `Initialize`.

Почему подсистема: данные и состояние должны жить вне акторов и переживать
перезагрузку уровня. GameInstanceSubsystem — стандартный способ хранить
глобальное состояние в UE.

### Камера
- **AConfiguratorCamera** — три режима (Genplan / Floor / Apartment), орбита,
  интерполяция позиции и поворота через таймер с фиксированной частотой.
- **AConfiguratorCameraController** — привязка ввода через Enhanced Input,
  клик по квартире через трассировку `ECC_Visibility`.

### UI (UMG)
- **UConfiguratorHUD** — контейнер, держит TopBar и ApartmentCard.
- **UTopBarWidget** — кнопки «Назад» / «Общий вид», текущий режим,
  панель этажей, чекбокс «Скрыть проданные».
- **UFloorPanelWidget / UFloorButtonWidget** — кнопки этажей.
- **UApartmentCardWidget** — всплывающая карточка выбранной квартиры
  с кнопкой «Забронировать».

Все взаимодействия UI ↔ подсистема — через делегаты. UI не хранит состояние,
только отражает то, что в подсистеме.

### 3D-представление
- **AApartmentActor** — куб квартиры, спавнится из GameMode. Подсветка
  выбранной и затемнение проданной квартиры через Overlay Material.

### Производительность
- Тяжёлых вычислений в Tick нет.
- Кеши считаются один раз в `Initialize`.
- Интерполяция камеры — таймер с фиксированным шагом, не Tick.

## Структура

Source/FloorConfigurator/
├── Base/           — PlayerController, GameMode
├── Camera/         — камера
├── Data/           — структуры данных, ConfigLoader
├── Interaction/    — ApartmentActor, ApartmentInterface
├── Subsystems/     — FloorConfiguratorSubsystem
└── UI/             — виджеты

## Сборка

1. Открыть `FloorConfigurator.uproject` в UE 5.7+.
2. Собрать C++ (Visual Studio / Rider).
3. Запустить уровень.
