[🇺🇦 Українська](#українська) | [🇬🇧 English](#english)

---

## українська

проект створювався чисто як експеримент, натхненний комп'ютерною графіклю на асемблері. 

1. спочатку я спробував реалізувати просто обертання куба в файлі `src\cube.asm`. вийшло не погано. цікаво як багато треба всього розписувати і як мало, порівняно з сучасними мовами програмування, така програма займає процесорного часу і оперативної па'мяті. мені вдалось оптимізувати до 0.2мб в ОП

2. далі я подумав чому б не зробити просту гру на асемблері. ну і прикрутити туди gemini API. так, через асемблер :)
    - ну і взагалі вийшло, насправді. через asm можна викликати windows служби, то фактично запит до нейронки був через них
    - перший прототип гри можна глянути в `src\maze.asm`

3. ну але я зрозумів що далеко на asm не вивезти. дуже складно на ньому масштабувати проекти. там більше треба паритись про деталі реалізації. дуже муторно. це не те що мені хочеться від програмування

    тому можна глянути спробу перейти на більш абстрактну мову прорамування C++. перший варіант реалізації лабіриту можна знайти в `src\maze.cpp`

4. потім я почав масштабувати гру, додаючи нові фічі. вороги, ключи, процедурна генерація, портали, повідомлення від "архітектора"
все це в папці `src-2\`

---

## english

this project was created purely as an experiment, inspired by computer graphics in assembly.

1. first i tried to implement a simple spinning cube in `src\cube.asm`. turned out not bad. it's crazy how much you have to write out manually and how little CPU time and RAM this kind of program takes compared to modern languages. i managed to optimize it down to 0.2mb in RAM

2. then i thought why not make a simple game in assembly. and hook up the gemini API to it. yep, through assembly :)
    - and it actually worked tbh. you can call windows services through asm, so basically the request to the neural network went through them
    - you can check out the first game prototype in `src\maze.asm`

3. but well i realized you can't go far with asm. scaling projects on it is super hard. you have to sweat too much over implementation details. such a drag. not exactly what i want from programming

    so you can see my attempt to switch to a more abstract programming language C++. the first version of the maze implementation can be found in `src\maze.cpp`

4. then i started scaling the game, adding new features. enemies, keys, procedural generation, portals, messages from the "architect"
all of this is in the `src-2\` folder