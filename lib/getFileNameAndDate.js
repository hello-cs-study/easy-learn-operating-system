import dayjs from 'dayjs';

export default function getFileNameAndDate(type) {
    const studyStartDate = dayjs('2025-02-25');
    const creationDate = dayjs().format('YYYY-MM-DD');
    const weekSinceStart = dayjs().diff(studyStartDate, 'week') + 1;

    switch (type) {
        case 'DIL':
            return {
                creationDate,
                weekSinceStart,
                fileName: `${creationDate}.md`
            };
        case 'PRESENTATION':
            return {
                creationDate: dayjs().format('YYYY-MM-DD'),
                weekSinceStart,
                fileName: `week_${weekSinceStart}.md`
            };
        default:
            throw new Error('TYPE NOT ALLOWED!');
    }
}
