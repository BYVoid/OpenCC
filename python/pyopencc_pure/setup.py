import setuptools
import shutil

with open("README.md", "r", encoding='utf-8') as fh:
    long_description = fh.read()

shutil.rmtree('pyopencc_pure/data/config', ignore_errors=True)
shutil.copytree('../../data/config', 'pyopencc_pure/data/config')
shutil.rmtree('pyopencc_pure/data/dictionary', ignore_errors=True)
shutil.copytree('../../data/dictionary', 'pyopencc_pure/data/dictionary')

fh_out = open('pyopencc_pure/data/dictionary/TWPhrases.txt', 'w+', encoding='utf-8')
for f_in in ['TWPhrasesIT.txt', 'TWPhrasesName.txt', 'TWPhrasesOther.txt']:
    with open('pyopencc_pure/data/dictionary/'+f_in, 'r', encoding='utf-8') as fh_in:
        fh_out.write(fh_in.read())
fh_out.close()

def reverse_dict(input_dict, output_dict):
    fh_out = open('pyopencc_pure/data/dictionary/'+output_dict, 'w+', encoding='utf-8')
    fh_in = open('pyopencc_pure/data/dictionary/'+input_dict, 'r', encoding='utf-8')
    for line in fh_in:
        chars = line.split('\t')
        if chars:
            char_to = chars[1].split(' ', 1)
            fh_out.write(char_to[0].replace('\n','')+'\t'+chars[0]+'\n')
    fh_in.close()
    fh_out.close()

reverse_dict('TWVariants.txt', 'TWVariantsRev.txt')
reverse_dict('TWPhrases.txt', 'TWPhrasesRev.txt')
reverse_dict('HKVariants.txt', 'HKVariantsRev.txt')

setuptools.setup(
    name="pyopencc_pure",
    version="0.0.1",
    author="BenFre",
    author_email="benfre@sohu.com",
    license='Apache Software License',
    description="pure python based on Open Chinese Convert",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/benfre/OpenCC/tree/benfre_master",
    packages=setuptools.find_packages(),
    classifiers=(
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: OS Independent",
    ),
    package_data={'pyopencc_pure': ['data/config/*.json',
                            'data/dictionary/*.txt']}
)
