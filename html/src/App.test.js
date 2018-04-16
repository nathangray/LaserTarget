import React from 'react';
import ReactDOM from 'react-dom';
import App from './App';
import APP from './Constants.js';
import * as GAME from './Game/constants.js';
import {shallow, mount, render} from 'enzyme'
import {Server} from 'mock-socket';

const mockServer = new Server(APP.WS_URL);

beforeEach(() => {
	// Reset mocked fetch calls
  fetch.resetMocks();
})

it('renders without crashing', () => {
  const div = document.createElement('div');
  ReactDOM.render(<App />, div);
});

it("Fetches state on #componentDidMount", async () => {

	fetch.mockResponse(JSON.stringify(GAME.State.PLAY), {status: 200})

	let app = shallow(<App />)

  await app.instance().componentDidMount()

	// Starts in idle
	expect(app.state()).toHaveProperty('state', GAME.State.IDLE);
	expect(app.state()).toHaveProperty('error', null);
});

it('Updates state according to what server says', () => {
	// Setup mocks
	fetch.mockResponse(JSON.stringify(GAME.State.PLAY), {status: 200});

	// Create app
	let app = shallow(<App />)
	spyOn(app.instance(), 'setState').and.callThrough();
	app.update();

	// Try state change - return promise to handle errors
	return app.instance().fetchState().then(() => {
			// Async check
			expect(app.state()).toHaveProperty('error', null);
			expect(fetch).toBeCalledWith('/state');
			expect(app.instance().setState).toBeCalled();
			expect(app.state()).toHaveProperty('state', GAME.State.PLAY);
		})
});
/* if this one is run, the next test to use mockServer fails
it('Connects to websocket on #componentDidMount', async (done) => {
	// Needed for initial connection
	fetch.mockResponse(JSON.stringify(GAME.State.PLAY), {status: 200});

	var worked = false;
	mockServer.on('connection', server => {
		worked = true;
	});

	// Create app
	let app = shallow(<App />);

	await app.instance().componentDidMount();

	setTimeout( () => {
		//mockServer.simulate('error');
		expect(app.state()).toHaveProperty('error', null);
		expect(worked).toBe(true);

		mockServer.stop(done);
	}, 100);
});
*/
it('Gets game type and settings from server', () => {

	// Create app
	let app = shallow(<App />);

	// Setup mocks
	fetch.mockResponse(JSON.stringify({
		type: GAME.Type.DOMINATION,
		config: {
			score_limit: 100
		}
	}), {status: 200});
	spyOn(app.instance(), 'setState').and.callThrough();
	app.update();

	// Try state change - return promise to handle errors
	return app.instance().fetchGame().then(() => {
			// Async check
			expect(fetch).toBeCalledWith('/game');
			expect(app.instance().setState).toBeCalled();
			expect(app.state()).toHaveProperty('game.type', GAME.Type.DOMINATION);
	});
});

it('Calls server to update state on change', async (done) => {
	// Needed for initial connection
	fetch.mockResponse(JSON.stringify(GAME.State.PLAY), {status: 200});

	// Setup mock
	var called = false;
	mockServer.on('message', (data) => {
		called = true;
		expect(JSON.parse(data)).toHaveProperty('state', GAME.State.PLAY);
	});


	// Create app
	const app = shallow(<App />)
	await app.instance().componentDidMount();

	// Try state change
	app.instance().stateChangeHandler(GAME.State.PLAY);

	// Check server got right state

	setTimeout(() => {
		// No errors
		expect(app.state()).toHaveProperty('error', null);
		expect(called).toBe(true);
		mockServer.stop(done);
	}, 100);
});
