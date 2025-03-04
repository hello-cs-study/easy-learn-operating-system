import * as fs from 'fs';
import * as path from 'path';
import * as readline from 'readline';
import { githubNames } from '../github-names.js';
import renderSelectGitHubNameMenu from './renderSelectGitHubNameMenu.js';
import getFileNameAndDate from './getFileNameAndDate.js';
import getPathInfo from './getPathInfo.js';

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
});

rl.on('line', (githubNameIndex) => {
  if (
    isNaN(githubNameIndex) || // isNaN이 true이면 숫자가 아닌 문자열
    Number(githubNameIndex) < 1 ||
    Number(githubNameIndex) > githubNames.length
  ) {
    throw new Error('번호는 1부터 10까지 입력해주세요!');
  }

  const memberGithubName = githubNames[githubNameIndex - 1]; // GitHub 닉네임
  const { creationDate, weekSinceStart } = getFileNameAndDate('DIL');
  const { basePath, mdPath } = getPathInfo(memberGithubName, 'DIL');

  const baseCheckPath = `${basePath}/week_${weekSinceStart}`;

  /** Initial Setting */
  if (!fs.existsSync(baseCheckPath)) {
    fs.mkdirSync(baseCheckPath);
    fs.mkdirSync(path.join(baseCheckPath, 'DIL'));
    fs.mkdirSync(path.join(baseCheckPath, 'presentation'));
  }

  const content = `
# DIL: 쉽게 배우는 운영체제

> 작성일: ${creationDate}<br/>
> 작성자: ${memberGithubName}<br/>
> 읽은 책 범위: 

---
  `.trim();

  if (!fs.existsSync(mdPath)) {
    fs.writeFileSync(mdPath, content, { recursive: true });
  }

  console.log('>>>> 😄 DIL FILE MAKE SUCCESS! 😄 <<<<');
  rl.close();
});

renderSelectGitHubNameMenu();
