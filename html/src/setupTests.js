// Setup for tests
global.fetch = require('jest-fetch-mock');

// For enzyme
import { configure } from 'enzyme';
import Adapter from 'enzyme-adapter-react-16';

configure({ adapter: new Adapter() });
