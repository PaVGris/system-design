import os
import fnmatch

def save_project_to_txt(output_filename='project_content.txt', path='.'):
    current_script_name = os.path.basename(__file__)
    
    ignored_dirs = {
        '.git', '__pycache__', 'build', 'dist', 'node_modules', 
        '.venv', 'venv', '.idea', '.ccache', '.vscode', 
        '.pytest_cache', 'Testing', '.cache'  # Добавил .cache
    }
    
    ignored_dir_patterns = ['cmake-build-*']

    # РАСШИРЕННЫЙ список игнорируемых расширений
    ignored_extensions = {
        # Картинки
        '.png', '.jpg', '.jpeg', '.gif', '.bmp', '.ico', '.svg',
        # Бинарники/скомпилированное
        '.pyc', '.exe', '.dll', '.so', '.o', '.a', '.lib', '.obj',
        # Архивы
        '.zip', '.tar', '.gz', '.7z', '.rar', '.bz2', '.xz',
        # Документы (бинарные)
        '.pdf', '.docx', '.xlsx', '.pptx', '.doc', '.xls',
        # Шрифты
        '.ttf', '.woff', '.woff2', '.eot',
        # Кэш и индексы (ВАЖНО!)
        '.idx', '.cache', '.pch', '.gch', '.obj', '.tlog',
        # Другие бинарные
        '.db', '.sqlite', '.bin', '.dat', '.class', '.jar',
        '.psd', '.ai', '.eps', '.mp3', '.mp4', '.avi', '.mov'
    }

    with open(output_filename, 'w', encoding='utf-8') as out_file:
        for root, dirs, files in os.walk(path):
            dirs[:] = [d for d in dirs if d not in ignored_dirs and 
                       not any(fnmatch.fnmatch(d, p) for p in ignored_dir_patterns)]
            
            for file in files:
                _, ext = os.path.splitext(file)
                ext = ext.lower()
                
                if (file == output_filename or 
                    file == current_script_name or 
                    file.startswith('.') or 
                    ext in ignored_extensions):  # Упростил проверку
                    continue
                    
                file_path = os.path.join(root, file)
                
                out_file.write(f"\n{'='*80}\n")
                out_file.write(f"ФАЙЛ: {file_path}\n")
                out_file.write(f"{'='*80}\n\n")
                
                try:
                    if os.path.getsize(file_path) > 1024 * 1024:
                        out_file.write("[Файл слишком большой (>1MB), пропущен]\n")
                        continue

                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                        # Доп. проверка: если много непечатаемых символов - пропускаем
                        if content and sum(c.isprintable() for c in content[:1000]) < 500:
                            out_file.write("[Файл похож на бинарный, пропущен]\n")
                        else:
                            out_file.write(content if content.strip() else "[Пустой файл]\n")
                except Exception as e:
                    out_file.write(f"[Ошибка чтения файла: {e}]\n")
                
                out_file.write("\n\n")

    print(f"Готово! Результат: {output_filename}")

if __name__ == "__main__":
    save_project_to_txt()